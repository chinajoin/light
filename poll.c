#include "poll.h"
#include "util.h"
#include "log.h"
#include "hash_table/hashtable.h"
#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>


/**
 * License GPLv3+
 */

//poll_event_element functions
/**
 * Function to allocate a new poll event element
 * @param fd the file descriptor to watch
 * @param events epoll events mask
 * @returns poll event element on success
 * @returns NULL on failure
 */
poll_element_t * poll_event_element_new(int fd, uint32_t events)
{
    info("Creating a new poll event element");

    poll_element_t *elem = calloc(1, poll_event_element_s);

    if (elem)
    {
        elem->fd = fd;
        elem->events = events;
    }
    return elem;
}


/**
 * Function to add a file descriptor to the event poll obeject
 * @note if add is performed on an fd already in poll_event, the flags are updated in the existing object
 * @param poll_event poll event object which fd has to be added
 * @param fd the file descriptor to be added
 * @param flags events flags from epoll
 * @param poll_element a poll event element pointer which is filled in by the function, set all function callbacks and cb_flags in this
 */
int poll_event_element_set(poll_event_t* poll_event, int fd, uint32_t flags, poll_element_t **poll_element)
{
    info("add poll event, event= %d, start lookup", fd);
    poll_element_t *elem = NULL;
    elem = (poll_element_t *) HT_LOOKUP(poll_event->table, &fd);
    if (elem)
    {
        info("poll event add, fd (%d) already added updating flags", fd);
        elem->events |= flags;
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = elem->events;
        *poll_element = elem;
        return epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    }
    else
    {
        elem = poll_event_element_new(fd, flags);
        info("poll event add, elem= %p", elem);
        if (HT_ADD(poll_event->table, &fd, elem))
        {
            // error in hash table
            return -1;
        }
        info("poll event add, fd(%d)", fd);
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = elem->events;
        *poll_element = elem;
        return epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    }
}

/**
 * Function to delete a poll event element
 * @param elem poll event element
 */
void poll_event_element_delete(poll_event_t* poll_event, poll_element_t * elem)
{
    info("Deleting a poll event element, fd=%d", elem->fd);
    HT_REMOVE(poll_event->table, &elem->fd);
    epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_DEL, elem->fd, NULL);
    shutdown(elem->fd, SHUT_RDWR);
    close(elem->fd);
    free(elem->st_req);
    free(elem);
}

// poll_event function
/**
 * Function to create a new poll event object
 * @param timeout timeout for the pollevent
 * @retunrs NULL on failure
 * @retunrs poll event object on sucess
 */
poll_event_t * poll_event_new(int timeout)
{
    poll_event_t * poll_event = calloc(1, poll_event_s);
    if (!poll_event)
    {
        info("calloc failed at poll_event");
        return NULL; // No Memory
    }
    poll_event->table = hash_table_new(MODE_VALUEREF);
    if (!poll_event->table)
    {
        free(poll_event);
        info("calloc failed at hashtble");
        return NULL;
    }
    poll_event->timeout = timeout;
    poll_event->epoll_fd = epoll_create(MAX_EVENTS);
    info("Created a new poll event");
    return poll_event;
}


/**
 * Function to delete poll event object
 * @param poll_event poll event object to be deleted
 */
void poll_event_destory(poll_event_t* poll_event)
{
    info("deleting a poll_event");
    hash_table_delete(poll_event->table);
    close(poll_event->epoll_fd);
    free(poll_event);
}

int poll_event_stop(poll_event_t* poll_event, int fd, uint32_t flags)
{
    poll_element_t * elem = NULL;
    elem = (poll_element_t *) HT_LOOKUP(poll_event->table, &fd);
    if(elem)
    {
        info("fd (%d) stop", fd);
        if ((elem->events & EV_READ) && (flags & EV_READ)) {
            elem->events &= (~EV_READ);
        }
        if ((elem->events & EV_WRITE) && (flags & EV_WRITE)) {
            elem->events &= (~EV_WRITE);
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = elem->events;
        if(-1 == epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_MOD, fd, &ev))
        {
            info("epoll_fd (%d) modify failed", poll_event->epoll_fd);
        }
        return 0;
    }
    else
    {
        info("fd (%d) not found", fd);
        return -1;
    }
}

/**
 * Function which processes the events from epoll_wait and calls the appropriate callbacks
 * @note only process events once if you need to use an event loop use poll_event_loop
 * @param poll_event poll event object to be processed
 */
int poll_event_process(poll_event_t * poll_event)
{
    struct epoll_event events[MAX_EVENTS];
    info("=======> process request start, on listensock(%d)...", poll_event->listen_sock);
    int fds = epoll_wait(poll_event->epoll_fd, events, MAX_EVENTS, poll_event->timeout);
    if (fds == -1)
    {
        info("event loop pwait");
        exit(EXIT_FAILURE);
    }
    info("event count: %d", fds);
	int i;
    /*struct epoll_event ev;*/
    for(i=0;i<fds;i++)
    {
        poll_element_t * poll_element  = NULL;
        if ((poll_element = (poll_element_t *) HT_LOOKUP(poll_event->table, &events[i].data.fd)) != NULL)
        {
            if(events[i].data.fd == poll_event->listen_sock)
			{
				// process connect
				info("process AC for event id(%d), sock(%d) and event(%d)", i, events[i].data.fd, events[i].events);
				poll_event->accept_callback(poll_event);
			}
			else
			{
				// process request
				// read
				if((events[i].events & EPOLLIN) && poll_element->read_callback)
				{
                    info("process READ for event id(%d), sock(%d) and event(%d)", i, events[i].data.fd, events[i].events);
					info("===> found EPOLLIN for event id(%d) and sock(%d)", i, events[i].data.fd);
					poll_element->read_callback(poll_event, poll_element);

                    //ev.data.fd = events[i].data.fd;
                    //ev.events = EPOLLOUT;
                    //epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                    poll_event_element_set(poll_event, events[i].data.fd, EPOLLOUT, &poll_element);
				}
				// write
				if((events[i].events & EPOLLOUT) && poll_element->write_callback)
				{
                    info("process WRITE for event id(%d), sock(%d) and event(%d)", i, events[i].data.fd, events[i].events);
					info("===> found EPOLLOUT for event id(%d) and sock(%d)", i, events[i].data.fd);
					poll_element->write_callback(poll_event, poll_element);
                    poll_event_element_delete(poll_event, poll_element);
				}
				// shutdown or error
				if((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
				{
					// close
					if(events[i].events & EPOLLRDHUP)
					{
						info("found EPOLLRDHUP for event id(%d) and sock(%d)", i, events[i].data.fd);
					}
					if(events[i].events & EPOLLERR)
					{
						info("found EPOLLERR for event id(%d) and sock(%d)", i, events[i].data.fd);
					}
					if(poll_element->close_callback)
					{
						poll_element->close_callback(poll_event, poll_element);
					}
				}
			}
        }
        else // not in table
        {
            info("WARNING: NOT FOUND hash table value for event id(%d) and sock(%d)", i, events[i].data.fd);
        }
    } // for
    info("=======> process request end");
    return 0;
}

/**
 * Function to start the event loop which monitors all fds and callbacks accordingly
 * @note event loop runs indefinitely and can only be stopped by timeout callback, so to process events only once use poll_event_process
 */
void poll_event_loop(poll_event_t* poll_event)
{
    info("Entering the main event loop for epoll lib");
	for(;;)
	{
		poll_event_process(poll_event);
	}
} 


#ifndef _POLL_H
#define _POLL_H

#include "hash_table/hashtable.h"
#include <sys/epoll.h>
#include <netinet/in.h>
#include "light.h"

#define MAX_EVENTS 100
#define CALLBACK(x) void (*x) (poll_event_t *, poll_element_t *)

#define EV_READ EPOLLIN
#define EV_WRITE EPOLLOUT

typedef struct poll_event_element poll_element_t;
typedef struct poll_event poll_event_t;

/**
 * License GPLv3+
 * @author Ankur Shrivastava
 * @struct poll_event_element "poll.h"
 * @brief a poll event element containing callbacks, user data and flags
 */
struct poll_event_element
{
    /** the file descriptor*/
	/** the client socket */
    int fd;
	/** the client addr */
	struct sockaddr_in addr;
    /** epoll events flags */
    uint32_t events;
    //read buf
    char buf[BUFFER_SIZE];
    //read pos
    int read_pos;
    //write pos
    int write_pos;
	//request info
	struct st_request *st_req;

    /** callback for read */
    CALLBACK(read_callback);
    /** callback for write */
    CALLBACK(write_callback);
    /** callback for close */
    CALLBACK(close_callback);
};
#define poll_event_element_s sizeof(poll_element_t)

/**
 * @struct poll_event "poll.h"
 * @brief poll event object 
 */
struct poll_event
{
    /** hash table of fds - poll event elements */
    hash_table_t *table;
    
    /** timeout duration */
    size_t timeout;
	/* socket fd */
	int listen_sock;
    /** epoll file descriptor*/
    int epoll_fd;
	/* deal user request */
    void (*accept_callback)(poll_event_t *);
};
#define poll_event_s sizeof(poll_event_t)

//poll_event_element functions
/**
 * Function to allocate a new poll event element
 * @param fd the file descriptor to watch
 * @param events epoll events mask
 * @returns poll event element on success
 * @returns NULL on failure
 */
poll_element_t * poll_event_element_new(int, uint32_t);

/**
 * Function to add a file descriptor to the event poll obeject
 * @note if add is performed on an fd already in poll_event, the flags are updated in the existing object
 * @param poll_event poll event object which fd has to be added
 * @param fd the file descriptor to be added
 * @param flags events flags from epoll
 * @param poll_element a poll event element pointer which is filled in by the function, set all function callbacks and cb_flags in this
 */
int poll_event_element_set(poll_event_t*, int, uint32_t, poll_element_t **);

/**
 * Function to delete a poll event element
 * @param elem poll event element
 */
void poll_event_element_delete(poll_event_t* poll_event, poll_element_t *);

// poll_event function
/**
 * Function to create a new poll event object
 * @param timeout timeout for the pollevent
 * @retunrs NULL on failure
 * @retunrs poll event object on sucess
 */
poll_event_t * poll_event_new(int);

/**
 * Function to delete poll event object
 * @param poll_event poll event object to be deleted
 */
void poll_event_destory(poll_event_t*);

int poll_event_stop(poll_event_t*, int, uint32_t);

/**
 * Function which processes the events from epoll_wait and calls the appropriate callbacks
 * @note only process events once if you need to use an event loop use poll_event_loop
 * @param poll_event poll event object to be processed
 */
int poll_event_process(poll_event_t *);

/**
 * Function to start the event loop which monitors all fds and callbacks accordingly
 * @note event loop runs indefinitely and can only be stopped by timeout callback, so to process the events only once use poll_event_process
 */
void poll_event_loop(poll_event_t*);
#define use_the_force(x) poll_event_loop(x)

#endif

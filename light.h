#ifndef _LIGHT_H
#define _LIGHT_H
/**************************************************************
 * Copyright (C) 2010-2015 All rights reserved.
 * @Version: 1.0
 * @Created: 2015-02-28 15:49
 * @Author: melanc - whdsmile@gmail.com qq:869314629
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <time.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define _GNU_SOURCE
#include <getopt.h>



/* constant define */
#define SERVER_NAME				"tmhttpd/1.0.0_alpha"
#define VERSION					"1.0.0 alpha"
#define MAXBUFSIZE				16*1024

#define BUFFER_SIZE				8192
#define REQUEST_MAX_SIZE		10240

/* configure constant */
#define IS_DEBUG				1					/* Is open debug mode */
#define IS_DAEMON				0					/* Is daemon running */
#define	IS_BROWSE				0					/* Is allow browse file/dir list */
#define IS_LOG					0					/* Is write access log */
#define DEFAULT_HTTP_PORT		8800					/* Server listen port */
#define DEFAULT_MAX_CLIENT		100					/* Max connection requests */
#define DEFAULT_DOCUMENT_ROOT	"./htdocs"				/* Web document root directory */
#define DEFAULT_DIRECTORY_INDEX	"index.html"		/* Directory default index file name */
#define DEFAULT_LOG_PATH		"/tmp/tmhttpd.log"	/* Access log path */


/* data struct define */
struct st_request {
	char *method;
	char *pathinfo;
	char *query;
	char **reqdata;
	char *protocal;
	char *file;
	char *realpath;
};

#endif
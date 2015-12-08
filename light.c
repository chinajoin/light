/**************************************************************
 * Copyright (C) 2010-2015 All rights reserved.
 * @Version: 1.0
 * @Created: 2015-02-28 15:49
 * @Author: melanc - whdsmile@gmail.com qq:869314629
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include "util.h"
#include "poll.h"
#include "log.h"
#include "light.h"


/* default configure */
static unsigned short g_is_debug	= IS_DEBUG;
static unsigned short g_is_daemon	= IS_DAEMON;
static unsigned short g_is_browse	= IS_BROWSE;
static unsigned short g_is_log		= IS_LOG;
static unsigned int g_port			= DEFAULT_HTTP_PORT;
static unsigned int g_max_client	= DEFAULT_MAX_CLIENT;
static char g_dir_index[32]			= DEFAULT_DIRECTORY_INDEX;
static char g_doc_root[256]			= DEFAULT_DOCUMENT_ROOT; 
static char g_log_path[256]			= DEFAULT_LOG_PATH;

/* Global variable */
static int g_log_fd					= 0;

// func declare

void accept_cb(poll_event_t *);

void read_cb (poll_event_t *, poll_element_t *);

void write_cb(poll_event_t *, poll_element_t *);

void close_cb (poll_event_t *, poll_element_t *);

/********************************
 *
 *   Http Server Basic Function
 *
 ********************************/

/**
 * Die alert message
 *
 */
void die(char *mess)
{
    perror(mess); 
    exit(1); 
}



/********************************
 *
 *   Http Server Core Function
 *
 ********************************/

 
/**
 * Usage message
 *
 */
void Usage(char *exefile)
{
	/* Print copyright information */
	fprintf(stderr, "#=======================================\n");
	fprintf(stderr, "# TieMa(Tiny&Mini) Http Server\n");
	fprintf(stderr, "# Version %s\n", VERSION);
	fprintf(stderr, "# \n");
	fprintf(stderr, "# heiyeluren <blog.csdn.net/heiyeshuwu>\n");
	fprintf(stderr, "#=======================================\n\n");
    fprintf(stderr, "Usage: %s [OPTION] ... \n", exefile);

	/* Print options information */
	fprintf(stderr, "\nOptions: \n\
  -D, --is-deubg	Is open debug mode, default No\n\
  -d, --is-daemon	Is daemon running, default No\n\
  -p, --port=PORT	Server listen port, default 80\n\
  -m, --max-client=SIZE	Max connection requests, default 100\n\
  -L, --is-log		Is write access log, default No\n\
  -l, --log-path=PATH	Access log path, default /tmp/tmhttpd.log\n\
  -b, --is-browse	Is allow browse file/dir list, default No\n\
  -r, --doc-root=PATH	Web document root directory, default programe current directory ./\n\
  -i, --dir-index=FILE	Directory default index file name, default index.html\n\
  -h, --help		Print help information\n");

	/* Print example information */
	fprintf(stderr, "\nExample: \n\
  %s -d -p 80 -m 128 -L -l /tmp/access.log -b -r /var/www -i index.html\n\
  %s -d -p80 -m128 -L -l/tmp/access.log -b -r/var/www -iindex.html\n\
  %s --is-daemon --port=80 --max-client=128 --is-log --log-path=/tmp/access.log --is-browse --doc-root=/var/www --dir-index=index.html\n\n", exefile, exefile, exefile);

}

/**
 * Output environment and configure information
 *
 */
void PrintConfig()
{
	fprintf(stderr, "===================================\n");
	fprintf(stderr, " tmhttpd Configure information\n");
	fprintf(stderr, "===================================\n");
	fprintf(stderr, "Is-Debug\t = %s\n", g_is_debug ? "Yes" : "No");
	fprintf(stderr, "Is-Daemon\t = %s\n", g_is_daemon ? "Yes" : "No");
	fprintf(stderr, "Port\t\t = %d\n", g_port);
	fprintf(stderr, "Max-Client\t = %d\n", g_max_client);
	fprintf(stderr, "Is-Log\t\t = %s\n", g_is_log ? "Yes" : "No");
	fprintf(stderr, "Log-Path\t = %s\n", g_log_path);
	fprintf(stderr, "Is-Browse\t = %s\n", g_is_browse ? "Yes" : "No");
	fprintf(stderr, "Doc-Root\t = %s\n", g_doc_root);
	fprintf(stderr, "Dir-Index\t = %s\n", g_dir_index);
	fprintf(stderr, "===================================\n\n");
}

/**
 * Log message
 *
 */
int WriteLog( const char *message )
{
	if ( !g_is_log ){
		fprintf(stderr, "%s", message);
		return 0;
	}
	if ( g_log_fd == 0 ){
		if ( (g_log_fd = open(g_log_path, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1 ){
			perror("open log file error");
			return -1;
		}
	}
	if (write(g_log_fd, message, strlen(message)) == -1){
		perror("write log error");
		return -1;
	}

	return 0;
}

/**
 * Send http header
 *
 */
int SendHeaders(int client_sock, int status, char *title, char *extra_header, char *mime_type, off_t length, time_t mod )
{
	info("start send headers...");
    time_t now;
    char timebuf[100], buf[BUFFER_SIZE], buf_all[REQUEST_MAX_SIZE];
	/* Make http head information */
    memset(buf, 0, strlen(buf));
    memset(buf_all, 0, strlen(buf_all));

    sprintf(buf, "%s %d %s\r\n", "HTTP/1.0", status, title);
    strcat(buf_all, buf);

    memset(buf, 0, strlen(buf));
    sprintf(buf, "Server: %s\r\n", SERVER_NAME);
    strcat(buf_all, buf);

    now = time( (time_t*) 0 );
    strftime( timebuf, sizeof(timebuf), "%a, %d %b %Y %H:%M:%S GMT", gmtime( &now ) );
    memset(buf, 0, strlen(buf));
    sprintf(buf, "Date: %s\r\n", timebuf);
    strcat(buf_all, buf);

    if (mime_type != (char*)0){
        memset(buf, 0, strlen(buf));
        sprintf(buf, "Content-Type: %s;charset=utf-8\r\n", mime_type);
        strcat(buf_all, buf);
    }
    if (length >= 0){
        memset(buf, 0, strlen(buf));
        sprintf(buf, "Content-Length: %ld\r\n", (long int)length);        
        strcat(buf_all, buf);
    }
    if (mod != (time_t) -1 ){
        memset(buf, 0, strlen(buf));
        strftime( timebuf, sizeof(timebuf), "%a, %d %b %Y %H:%M:%S GMT", gmtime( &mod ) );
        sprintf(buf, "Last-Modified: %s\r\n", timebuf );
        strcat(buf_all, buf);
    }

	// 判断是否为空字符串
    if (extra_header != (char*)0 && *extra_header != '\0'){
        memset(buf, 0, strlen(buf));
        sprintf(buf, "%s\r\n", extra_header);
        strcat(buf_all, buf);
    }

    memset(buf, 0, strlen(buf));
    sprintf(buf, "Connection: close\r\n\r\n");
    strcat(buf_all, buf);

	/* Debug message */
	if ( g_is_debug ){
		fprintf(stderr, "[ Response ]\n");
		fprintf(stderr, "%s", buf_all);
	}

	/* Write http header to client socket */
    if( -1 == write(client_sock, buf_all, strlen(buf_all))){
		info("can not write headers to client(%d)", client_sock);
		return -1;
	}
	return 0;
} 


/**
 * Send http error message 
 * 
 * @param int status
 * @param char* title
 * @param char* extra
 */
int SendError(int client_sock, int status, char *title, char *extra_header, char *text )
{
    char buf[BUFFER_SIZE], buf_all[REQUEST_MAX_SIZE];

	/* Send http header */
    int err = SendHeaders( client_sock, status, title, extra_header, "text/html", -1, -1 );
	if(err < 0)
	{
		return -1;
	}

	/* Send html page */
    memset(buf, 0, strlen(buf));
    sprintf(buf, "<html>\n<head>\n<title>%d %s - %s</title>\n</head>\n<body>\n<h2>%d %s</h2>\n", status, title, SERVER_NAME, status, title);
    strcat(buf_all, buf);

    memset(buf, 0, strlen(buf));
    sprintf(buf, "%s\n", text);
    strcat(buf_all, buf);

    memset(buf, 0, strlen(buf));
    sprintf(buf, "\n<br /><br /><hr />\n<address>%s</address>\n</body>\n</html>\n", SERVER_NAME);
    strcat(buf_all, buf);

	/* Write client socket */
    if(-1 == write(client_sock, buf_all, strlen(buf_all))){
		fprintf(stderr, "can not write error to client");
		return -1;
	}
	return 0;
}

/**
 * Send file contents to client
 *
 */
int SendFile( int client_sock, char *filename, char *pathinfo )
{
    char buf[128], contents[8192], mime_type[128];
    int fd;
	size_t file_size;
    /* Get mime type & send http header information */
    mime_content_type(filename, mime_type);
    int err = SendHeaders( client_sock, 200, "OK", "", mime_type, filesize(filename), 0);
	if(err < 0)
	{
		return -1;
	}
	else
	{
		/* Open file */
		if ( (fd = open(filename, O_RDONLY)) == -1 ){
			memset(buf, '\0', sizeof(buf));
			sprintf(buf, "Something unexpected went wrong read file %s.", pathinfo);
			SendError( client_sock, 500, "Internal Server Error", "", buf);
			return -1;
		}

		/* Read static file write to client socket (File size less 8192 bytes) */
		file_size = filesize(filename);
		if ( file_size < 8192){
			if( -1 == read(fd, contents, 8192)){
				info("can not read file.");
				return -1;
			}
			if( -1 == write( client_sock, contents, file_size )){
				info("can not write file1 to client.");
				return -1;
			}

		} else {
			while ( read(fd, contents, 8192) > 0 ){
				if( -1 == write( client_sock, contents, 8192 )){
					info("can not write file2 to client.");
					return -1;
				}
				memset(contents, '\0', sizeof(contents));
			}
		}

		return 0;
	}
}

/**
 * Send directory index list to client
 *
 */
int SendDirectory( int client_sock, char *path, char *pathinfo )
{
	size_t file_size;
	char msg[128], buf[10240], tmp[1024], tmp_path[1024];
	DIR *dp;
	struct dirent *node;

	/* Open directory */
	if ( (dp = opendir(path)) == NULL ){
		memset(buf, 0, sizeof(msg));
		sprintf(msg, "Something unexpected went wrong browse directory %s.", pathinfo);
		SendError( client_sock, 500, "Internal Server Error", "", msg);
		return -1;
	}

	/* Check pathinfo is valid */
	memset(tmp_path, 0, sizeof(tmp_path));
	substr(pathinfo, -1, 1, tmp_path);
	if ( strcmp(tmp_path, "/") == 0){
		memset(tmp_path, 0, sizeof(tmp_path));
		strcpy(tmp_path, pathinfo);
	} else {
		memset(tmp_path, 0, sizeof(tmp_path));
		strcpy(tmp_path, pathinfo);	
		strcat(tmp_path, "/");
	}

	/* Fetch directory list */
	sprintf( tmp, "<html>\n<head>\n<title>Browse directory %s - %s</title>\n</head>\n<body><h3>Directory %s</h3>\n<hr />\n<ul>\n", pathinfo, SERVER_NAME, pathinfo );
	strcat(buf, tmp);

	while ( (node = readdir(dp)) != NULL){
		if ( strcmp(node->d_name, ".") != 0){
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "\t<li><a href=\"%s%s\">%s</a></li>\n", tmp_path, node->d_name, node->d_name);
			strcat(buf, tmp);
		}
	}
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "\n</ul>\n<br /><hr />\n<address>%s</address>\n</body>\n</html>\n", SERVER_NAME);
    strcat(buf, tmp);

	file_size = strlen(buf);
	int err = SendHeaders( client_sock, 200, "OK", "", "text/html", file_size, 0 );
	if(err < 0)
	{
		return -1;
	}
	else
	{
		if( -1 == write( client_sock, buf, file_size )){
			fprintf(stderr, "can not write dir to client.");
			return -1;
		}
		return 0;
	}
}


/**
 * Process request
 *
 */
int ProcRequest(poll_event_t * poll_event, int client_sock, poll_element_t * elem)
{
	char buf[128];

	/* File is exist or has access permission */
	if ( !file_exists( elem->st_req->realpath ) ){
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "File %s not found.", elem->st_req->realpath);
		info("File %s not found.", elem->st_req->realpath);
		SendError( client_sock, 404, "Not Found", "", buf);
		/*poll_event_element_delete(poll_event, elem);*/
		return -1;
	}
	if ( access(elem->st_req->realpath, R_OK) != 0 ){
		memset(buf, 0, sizeof(buf));
		info("File %s is protected.", elem->st_req->realpath);
		sprintf(buf, "File %s is protected.", elem->st_req->realpath);
		SendError( client_sock, 403, "Forbidden", "", buf);
		/*poll_event_element_delete(poll_event, elem);*/
		return -1;
	}

	/* Check target is regular file or directory */
	if ( is_file(elem->st_req->realpath) == 1 ){
		SendFile( client_sock,  elem->st_req->realpath, elem->st_req->pathinfo );
		/*poll_event_element_delete(poll_event, elem);*/
		return 0;
	} else if ( is_dir(elem->st_req->realpath) == 1 ){ 
		/* Is a directory choose browse dir list */
		if ( g_is_browse ){
			SendDirectory( client_sock, elem->st_req->realpath, elem->st_req->pathinfo );
			/*poll_event_element_delete(poll_event, elem);*/
			return 0;
		} else {
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "File %s is protected.", elem->st_req->realpath);
			SendError( client_sock, 403, "Forbidden", "", buf);
			/*poll_event_element_delete(poll_event, elem);*/
			return -1;
		}

	} else {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "File %s is protected.", elem->st_req->realpath);
		SendError( client_sock, 403, "Forbidden", "", buf);		
		/*poll_event_element_delete(poll_event, elem);*/
		return -1;
	}

	return 0;
}




/**
 * Parse client request
 *
 */
int ParseRequest( int client_sock, struct sockaddr_in client_addr, char *req, struct st_request *st_req)
{
	char **buf, **method_buf, **query_buf, currtime[32], cwd[1024], pathinfo[512], file[256], log[1024];
	int line_total, method_total, query_total, i;

	// 存放请求的相关信息：method，path，head，etc
	/* Split client request */
	getdate(currtime);

	// http协议为文本格式，以CRLF（回车换行）分割
	// 第一行： 起始行，包含method、path、http version 
	// 第二行～第N行：首部，包含各种头信息 如Content-Type
	// 最后：实体，首部和实体之间由空行分隔，存放数据，如POST提交
	explode(req, '\n', &buf, &line_total);

	/* Print log message  */
	memset(log, 0, sizeof(log));
	sprintf(log, "[%s] %s %s\n", currtime, inet_ntoa(client_addr.sin_addr), buf[0]);
	WriteLog(log);

	/* Check request is empty */
	if (strcmp(buf[0], "\n") == 0 || strcmp(buf[0], "\r\n") == 0){
		//SendError( client_sock, 400, "Bad Request", "", "Can't parse request." );
	}

	/* Check method is implement */
	//起始行: method、path、http version
	explode(buf[0], ' ', &method_buf, &method_total);
	if ( strcmp( strtolower(method_buf[0]), "get") != 0 &&  strcmp( strtolower(method_buf[0]), "head") != 0 ){
		//SendError( client_sock, 501, "Not Implemented", "", "That method is not implemented." );
	}
	// 获取get参数
	explode(method_buf[1], '?', &query_buf, &query_total);

	// 获取数据实体
	int j, k, l;
	int data_num = 0;
	int is_data = -1;
	for(j=0; j<line_total; j++){
		if(strlen(buf[j]) == 1){
			data_num = j+1;
			is_data = 0;
			break;
		}
	}
	char **req_data = (char **)malloc((line_total-data_num) * sizeof(char *));
	l=0;
	if(is_data == 0 && data_num != 0){
		for(k=data_num; k<line_total; k++, l++){
			int tmp_len = strlen(buf[k]);
			req_data[l] = (char *)malloc(tmp_len);
			memcpy(req_data[l], buf[k], tmp_len);
		}
	}

	/* Make request data */	
	// 保存当前目录名
	if(! getcwd(cwd, sizeof(cwd))){
		fprintf(stderr, "can not copy the path");
	}
	strcpy(pathinfo, query_buf[0]);
	substr(query_buf[0], strrpos(pathinfo, '/')+1, 0, file);
	
	// 绝对路径
	strcat(cwd, pathinfo);

	char protocal[16], query[256];
	memset(protocal, 0, strlen(protocal));
	memset(query, 0, strlen(query));
	strcpy(protocal, strtolower(method_buf[2]));
	strcpy(query, (query_total == 2 ? query_buf[1] : ""));

	memcpy(st_req->method, method_buf[0], sizeof(*method_buf[0]));
	memcpy(st_req->pathinfo, pathinfo, sizeof(pathinfo));
	memcpy(st_req->query, query, sizeof(query));
	memcpy(st_req->protocal, cwd, sizeof(cwd));
	memcpy(st_req->file, file, sizeof(file));
	memcpy(st_req->realpath, cwd, sizeof(cwd));
	memcpy(*st_req->reqdata, *req_data, strlen(*req_data));

	/* Is a directory pad default index page */
	if ( is_dir(cwd) ){
		strcat(cwd, g_dir_index);
		if ( file_exists(cwd) ){
			memcpy((st_req)->realpath, cwd, strlen(cwd));
		}
	}
	
	/* Debug message */
	if ( g_is_debug ){
		fprintf(stderr, "[ Request header ]");
		for(i=0; i<line_total; i++){
			if(buf[i] != '\0'){
				fprintf(stderr, "%s\n", buf[i]);
			}
		}
		if(req_data != (char **)0){
			fprintf(stderr, "[ Request data ]");
			for(i=0; i<l; i++){
				fprintf(stderr, "%s\n", req_data[i]);
			}
		}
	}

	return 0;
}


void read_cb (poll_event_t * poll_event, poll_element_t * elem)
{
    // NOTE -> read is also invoked on accept and connect
    // we just read data and print
	char *buf = elem->buf;
	info("[===002===]start read, fd=%d", elem->fd);
	int len = 0;
	int read_complete = 0;
	// 0: end; -1: error
	for(;;)
	{
		len = read(elem->fd, buf + elem->read_pos, BUFFER_SIZE - elem->read_pos);
		info("read %d byte", len);
		if(len > 0)
		{
			read_complete = (strstr(buf + elem->read_pos, "\n\n") != 0) || (strstr(buf + elem->read_pos, "\r\n\r\n") != 0);
			elem->read_pos += len;
			if(read_complete)
			{
				break;
			}
		}
		else if(len == -1)
		{
			if (errno != EAGAIN)
			{
				info("read: an error,errno=%d", errno);
				return;
			}
			else
			{
				//这个地方应该是返回，等下一次触发继续读
				return;
			}
		}
		else if(len == 0)
		{
			info("read: done");
			return;
		}
	}
	
	// if we actually get data print it
	if(read_complete)
	{
		//handle request
		info("read done, next");
		ParseRequest( elem->fd, elem->addr, buf, elem->st_req);
		info("parse done, realpath:%s", elem->st_req->realpath);
		// register write callback
		/*unsigned flags = EPOLLOUT;*/
		/*poll_event_element_set(poll_event, elem->fd, flags, &elem);*/
	}
}

void write_cb(poll_event_t * poll_event, poll_element_t * elem)
{
	info("[===003===]start write, fd=%d", elem->fd);
	ProcRequest(poll_event, elem->fd, elem);
}


void close_cb (poll_event_t * poll_event, poll_element_t * elem)
{
    // close the socket, we are done with it
    /*poll_event_element_delete(poll_event, elem);*/
}

void accept_cb(poll_event_t * poll_event)
{
    // accept the connection 
    int conn_fd = accept(poll_event->listen_sock, (struct sockaddr*)NULL, NULL);
    fcntl(conn_fd, F_SETFL, O_NONBLOCK);

    info("[===001===]start accept, fd=%d", conn_fd);
    // set flags to check 
    uint32_t flags = EPOLLIN;
    poll_element_t *p;
    // add file descriptor to poll event
    poll_event_element_set(poll_event, conn_fd, flags, &p);
    // set function callbacks 
    p->read_callback = read_cb;
    p->write_callback = write_cb;
    p->close_callback = close_cb;
}


/**
 * Initialize server socket listen
 * socket()
 * bind()
 * listen()
 * accept
 *
 */
void InitServerListen( unsigned int port, unsigned int max_client )
{
    int listensock;
	// sockaddr_in与sockaddr等价，不直接对sockaddr操作
    struct sockaddr_in server_addr;
	char currtime[32];

    /* Create the TCP socket */
	// 创建socket
    if ((listensock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        die("Failed to create socket");
    }

    /* Construct the server sockaddr_in structure */
	// 清空结构体
    memset(&server_addr, 0, sizeof(server_addr));       /* Clear struct */
	// 一般都为 AF_INET
    server_addr.sin_family = AF_INET;                  /* Internet/IP */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
    server_addr.sin_port = htons(port);          /* server port */

    /* Bind the server socket */
	// 绑定端口
    if (bind(listensock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        die("Failed to bind the server socket");
    }
    /* Listen on the server socket */
	// 监听端口
    if (listen(listensock, max_client) < 0){
        die("Failed to listen on server socket");
    }

	/* Print listening message */
	getdate(currtime);
	info("[%s] Start server listening at port %d ...", currtime, port);
	info("[%s] Waiting client connection ...", currtime);


    /* Run until cancelled */
	fcntl(listensock, F_SETFL, O_NONBLOCK);
	info("listen: fd=%d", listensock);
    // create a poll event object, with time out of 1 sec
	/*
	struct poll_event_t
	{
		hash_table_t *table;
		size_t timeout;
		int epoll_fd;
	}; */

    poll_event_t *pe = poll_event_new(-1);
	pe->listen_sock = listensock;
    pe->accept_callback = accept_cb;

	poll_element_t *p;
    // add sock to poll event
    poll_event_element_set(pe, listensock, EPOLLIN, &p);
    // start the event loop
    use_the_force(pe);

}

/** 
 * Parse cmd options
 *
 */
int ParseOptions( int argc, char *argv[] )
{
	int opt;
	struct option longopts[] = {
		{ "is-debug",	0, NULL, 'D' },
		{ "is-daemon",	0, NULL, 'd' },
		{ "port",		1, NULL, 'p' },
		{ "max-client",	1, NULL, 'm' },
		{ "is-log",		0, NULL, 'L' },
 		{ "log-path",	1, NULL, 'l' },
		{ "is-browse",	0, NULL, 'b' },
		{ "doc-root",	1, NULL, 'r' },
		{ "dir-index",	1, NULL, 'i' },
		{ "help",		0, NULL, 'h' },
		{ 0,			0, 0,	 0   }
	};

	/* Parse every options */
	// ":" 后面可以传参数，例如：--port 8080
	while ( (opt = getopt_long(argc, argv, ":Ddp:m:Ll:br:i:h", longopts, NULL)) != -1){
		switch (opt){
			case 'h': 
				// 打印帮助说明，并退出
				Usage(argv[0]); 
				return(-1);
				break;
			case 'D': g_is_debug = 1; break;
			case 'd': g_is_daemon = 1; break;
			case 'p':
				g_port = atoi(optarg);
				if ( g_port < 1 || g_port > 65535 ){
					fprintf(stderr, "Options -p,--port error: input port number %s invalid, must between 1 - 65535.\n\n", optarg);
					return(-1);
				}
				break;
			case 'm':
				g_max_client = atoi(optarg);
				if ( !isdigit(g_max_client) ){
					fprintf(stderr, "Options -m,--max-client error: input clients %s invalid, must number, proposal between 32 - 2048.\n\n", optarg);
					return(-1);
				}
				break;
			case 'L': g_is_log = 1; break;
			case 'l':
				strcpy(g_log_path, optarg);
				if ( !file_exists(g_log_path) || !is_dir(g_log_path) ){
					fprintf(stderr, "Options -l,--log-path error: input path %s not exist or not a directory.\n\n", optarg);
					return(-1);					
				}
				break;
			case 'b': g_is_browse = 1; break;
			case 'r':
				strcpy(g_doc_root, optarg);
				if ( !file_exists(g_doc_root) || !is_dir(g_doc_root) ){
					fprintf(stderr, "Options -l,--log-path error: input path %s not exist or not a directory.\n\n", optarg);
					return(-1);					
				}
				break;
			case 'i':
				strcpy(g_dir_index, optarg);
				break;
		}
	}

	return(0);
}



/********************************
 *
 *   Http Server running
 *
 ********************************/

/**
 * Main function 
 *
 */
int main( int argc, char *argv[] )
{

	/* Parse cli input options */
	//解析命令行参数
	if ( argc > 1 ){
		if ( ParseOptions( argc, argv ) != 0 ){
			exit(-1);
		}
	}

	/* Set work directory */
	//设置工作目录
	if(chdir(g_doc_root) == -1){
		exit(-1);
	}
	
	/* Set is daemon mode run */
	//是否以守护进程方式运行（父进程退出）
	/*
	if ( g_is_daemon ){
		pid_t pid;
		if ( (pid = fork()) < 0 ){
			die("daemon fork error");
		} else if ( pid != 0){
			exit(1);
		}
	} */

	/* Debug mode out configure information */
	//是否打印调试信息
	if ( g_is_debug ){
		PrintConfig();
	}

	/* Start server listen */
	//启动http服务器
	InitServerListen( g_port, g_max_client );

	return 0;	
}


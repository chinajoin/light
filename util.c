#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>

#include <sys/stat.h>
#include <fcntl.h>

/**
 * substr - Sub string from pos to length
 * 字符串截取函数，返回截取的字符串
 *
 */
char *substr( const char *s, int start_pos, int length, char *ret ){
    char buf[length+1];
    int i, j, end_pos;
	int str_len = strlen(s);

    if (str_len <= 0 || length < 0){
		return "";        
	}
	if (length == 0){
		length = str_len - start_pos;
	}
	if(start_pos < 0){
		start_pos += str_len;
	}else if(start_pos > 0){
		start_pos--;
	}
	end_pos = start_pos + length;

    for(i=start_pos, j=0; i<end_pos && j<=length; i++, j++){
		buf[j] = s[i];        
	}
    buf[length] = '\0';
    strcpy(ret, buf);

    return(ret);
}

/**
 * explode -  separate string by separator
 *
 * @param string from - need separator 
 * @param char delim - separator
 * @param pointarray to - save return separate result
 * @param int item_num - return sub string total
 * 
 * @include stdlib.h 
 * @include string.h
 *
 * @example
 * char *s, **r;
 * int num;
 * explode(s, '\n', &r, &num);
 * for(i=0; i<num; i++){
 *     printf("%s\n", r[i]);
 * }
 * 
 */
void explode(char *from, char delim, char ***to, int *item_num){
    int i, j, k, n, temp_len;
    int max_len = strlen(from) + 1;
    char buf[max_len], **ret;
       
	// 计算行数
    for(i=0, n=1; from[i]!='\0'; i++){
        if (from[i] == delim) n++;
    }
    
    ret = (char **)malloc(n*sizeof(char *));
    for (i=0, k=0; k<n; k++){
        memset(buf, 0, max_len);     
        for(j=0; from[i]!='\0' && from[i]!=delim; i++, j++)
            buf[j] = from[i];
        i++;
        temp_len = sizeof(buf);
        ret[k] = malloc(temp_len);
        memcpy(ret[k], buf, temp_len);
    } 
    *to = ret;
    *item_num = n;
}

/**
 * strtolower - string to lowner
 *
 */
char *strtolower( char *s ){
	int i, len = sizeof(s);
	for( i = 0; i < len; i++ ){
		/*s[i] = ( s[i] >= 'A' && s[i] <= 'Z' ? s[i] + 'a' - 'A' : s[i] );*/
        s[i] = tolower(s[i]);
	}
	return(s);
}

/**
 * strtoupper - string to upper
 *
 */
char *strtoupper( char *s ){
	int i, len = sizeof(s);
	for( i = 0; i < len; i++ ){
		/*s[i] = ( s[i] >= 'a' && s[i] <= 'z' ? s[i] + 'A' - 'a' : s[i] );*/
        s[i] = toupper(s[i]);
	}
	return(s);
}

/**
 * strpos - find char at string position
 *
 */
int strpos (const char *s, char c){
	int i, len;
	if (!s || !c) return -1;
	len = strlen(s);
	for (i=0; i<len; i++){
		if (s[i] == c) return i;
	}
	return -1;	
}

/**
 * strrpos - find char at string last position
 *
 */
int strrpos (const char *s, char c){
	int i, len;
	if (!s || !c) return -1;
	len = strlen(s);
	for (i=len; i>=0; i--){
		if (s[i] == c) return i;
	}
	return -1;
}




/**
 * trim - strip left&right space char
 *
 */
char *trim( char *s ){   
    int l;   
    for( l=strlen(s); l>0 && isspace((u_char)s[l-1]); l-- ){
		s[l-1] = '\0';   
	}
    return(s);   
}   


/**
 * ltrim - strip left space char
 *
 */
char *ltrim( char *s ){   
    char *p;   
    for(p=s; isspace((u_char)*p); p++ );   
    if( p!=s ) strcpy(s, p);   
    return(s);   
}   


/**
 *  filesize - get file size
 */
long filesize(const char *filename){
    struct stat buf;
    if (!stat(filename, &buf)){
        return buf.st_size;
    }
    return 0;
}

/**
 * file_exists - check file is exist
 */
int file_exists(const char *filename){
    struct stat buf;
    if (stat(filename, &buf) < 0){
        if (errno == ENOENT){
            return 0;
        }
    }
    return 1;
}

/**
 * file_get_contents - read file contents
 *
 */
int file_get_contents( const char *filename, size_t filesize, char *ret, off_t length ){
	if ( !file_exists(filename) || access(filename, R_OK)!=0 )	return -1;

	int fd;
	char buf[filesize];

	if ( (fd = open(filename, O_RDONLY)) == -1) return -1;
	length = ( length > 0 ? length : filesize);
	if(read(fd, buf, length) == -1){
		fprintf(stderr, "can not read file.");
	}
	strcpy(ret, buf);
	close(fd);

	return 0;
}

/**
 * is_dir - check file is directory
 * 
 */
int is_dir(const char *filename){
	struct stat buf;
	if ( stat(filename, &buf) < 0 ){
		return -1;
	}
	if (S_ISDIR(buf.st_mode)){
		return 1;
	}
	return 0;
}

/**
 * is_file - check file is regular file
 * 
 */
int is_file(const char *filename){
	struct stat buf;
	if ( stat(filename, &buf) < 0 ){
		return -1;
	}
	if (S_ISREG(buf.st_mode)){
		return 1;
	}
	return 0;
}

/**
 * Fetch current date tme
 *
 */
void getdate(char *s){
	//char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep);
	sprintf(s, "%d-%d-%d %d:%d:%d",(1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
} 

/**
 * Get MIME type header
 *
 */
void mime_content_type( const char *name, char *ret ){
    char *dot, *buf; 

    dot = strrchr(name, '.'); 

	/* Text */
    if ( strcmp(dot, ".txt") == 0 ){
        buf = "text/plain";
    } else if ( strcmp( dot, ".css" ) == 0 ){
        buf = "text/css";
    } else if ( strcmp( dot, ".js" ) == 0 ){
        buf = "text/javascript";
    } else if ( strcmp(dot, ".xml") == 0 || strcmp(dot, ".xsl") == 0 ){
        buf = "text/xml";
    } else if ( strcmp(dot, ".xhtm") == 0 || strcmp(dot, ".xhtml") == 0 || strcmp(dot, ".xht") == 0 ){
        buf = "application/xhtml+xml";
    } else if ( strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0 || strcmp(dot, ".shtml") == 0 || strcmp(dot, ".hts") == 0 ){
        buf = "text/html";

	/* Images */
    } else if ( strcmp( dot, ".gif" ) == 0 ){
        buf = "image/gif";
    } else if ( strcmp( dot, ".png" ) == 0 ){
        buf = "image/png";
    } else if ( strcmp( dot, ".bmp" ) == 0 ){
        buf = "application/x-MS-bmp";
    } else if ( strcmp( dot, ".jpg" ) == 0 || strcmp( dot, ".jpeg" ) == 0 || strcmp( dot, ".jpe" ) == 0 || strcmp( dot, ".jpz" ) == 0 ){
        buf = "image/jpeg";

	/* Audio & Video */
    } else if ( strcmp( dot, ".wav" ) == 0 ){
        buf = "audio/wav";
    } else if ( strcmp( dot, ".wma" ) == 0 ){
        buf = "audio/x-ms-wma";
    } else if ( strcmp( dot, ".wmv" ) == 0 ){
        buf = "audio/x-ms-wmv";
    } else if ( strcmp( dot, ".au" ) == 0 || strcmp( dot, ".snd" ) == 0 ){
        buf = "audio/basic";
    } else if ( strcmp( dot, ".midi" ) == 0 || strcmp( dot, ".mid" ) == 0 ){
        buf = "audio/midi";
    } else if ( strcmp( dot, ".mp3" ) == 0 || strcmp( dot, ".mp2" ) == 0 ){
        buf = "audio/x-mpeg";
	} else if ( strcmp( dot, ".rm" ) == 0  || strcmp( dot, ".rmvb" ) == 0 || strcmp( dot, ".rmm" ) == 0 ){
        buf = "audio/x-pn-realaudio";
    } else if ( strcmp( dot, ".avi" ) == 0 ){
        buf = "video/x-msvideo";
    } else if ( strcmp( dot, ".3gp" ) == 0 ){
        buf = "video/3gpp";
    } else if ( strcmp( dot, ".mov" ) == 0 ){
        buf = "video/quicktime";
    } else if ( strcmp( dot, ".wmx" ) == 0 ){
        buf = "video/x-ms-wmx";
	} else if ( strcmp( dot, ".asf" ) == 0  || strcmp( dot, ".asx" ) == 0 ){
        buf = "video/x-ms-asf";
    } else if ( strcmp( dot, ".mp4" ) == 0 || strcmp( dot, ".mpg4" ) == 0 ){
        buf = "video/mp4";
	} else if ( strcmp( dot, ".mpe" ) == 0  || strcmp( dot, ".mpeg" ) == 0 || strcmp( dot, ".mpg" ) == 0 || strcmp( dot, ".mpga" ) == 0 ){
        buf = "video/mpeg";

	/* Documents */
    } else if ( strcmp( dot, ".pdf" ) == 0 ){
        buf = "application/pdf";
    } else if ( strcmp( dot, ".rtf" ) == 0 ){
        buf = "application/rtf";
	} else if ( strcmp( dot, ".doc" ) == 0  || strcmp( dot, ".dot" ) == 0 ){
        buf = "application/msword";
	} else if ( strcmp( dot, ".xls" ) == 0  || strcmp( dot, ".xla" ) == 0 ){
        buf = "application/msexcel";
	} else if ( strcmp( dot, ".hlp" ) == 0  || strcmp( dot, ".chm" ) == 0 ){
        buf = "application/mshelp";
	} else if ( strcmp( dot, ".swf" ) == 0  || strcmp( dot, ".swfl" ) == 0 || strcmp( dot, ".cab" ) == 0 ){
        buf = "application/x-shockwave-flash";
	} else if ( strcmp( dot, ".ppt" ) == 0  || strcmp( dot, ".ppz" ) == 0 || strcmp( dot, ".pps" ) == 0 || strcmp( dot, ".pot" ) == 0 ){
        buf = "application/mspowerpoint";

	/* Binary & Packages */
    } else if ( strcmp( dot, ".zip" ) == 0 ){
        buf = "application/zip";
    } else if ( strcmp( dot, ".rar" ) == 0 ){
        buf = "application/x-rar-compressed";
    } else if ( strcmp( dot, ".gz" ) == 0 ){
        buf = "application/x-gzip";
    } else if ( strcmp( dot, ".jar" ) == 0 ){
        buf = "application/java-archive";
	} else if ( strcmp( dot, ".tgz" ) == 0  || strcmp( dot, ".tar" ) == 0 ){
        buf = "application/x-tar";
	} else {
		buf = "application/octet-stream";
	}
	strcpy(ret, buf);
}

void INFO(char * format)
{
    time_t t;
    char time_buf[100];
    strftime(time_buf, 100, "%Y%m%d%H%M%S", localtime(&t));
    fprintf(stderr,"[%s] %s:%d:%s -> %s\n", time_buf, __FILE__, __LINE__, __func__, format);
}



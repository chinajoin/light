#ifndef _UTIL_H
#define _UTIL_H

#include <sys/types.h>
#include <time.h>

char *substr( const char *, int, int, char *);

void explode(char *, char, char ***, int *);

char *strtolower( char *);

char *strtoupper( char *);

int strpos (const char *, char);

int strrpos (const char *, char);

char *trim( char *);

char *ltrim( char *);

long filesize(const char *);

int file_exists(const char *);

int file_get_contents( const char *, size_t, char *, off_t);

int is_dir(const char *);

int is_file(const char *);

void getdate(char *);

void mime_content_type( const char *, char *);

#endif

#ifndef _MONO_STDIO_H
#define _MONO_STDIO_H

#include "types.h"

/* Standard I/O for MonoOS userspace programs */

/* Formatted output */
int    printf(const char* fmt, ...);
int    putchar(int c);
int    puts(const char* str);

/* Character input */
int    getchar(void);

/* String formatting */
int    snprintf(char* buf, size_t size, const char* fmt, ...);

/* File I/O */
#define EOF (-1)

typedef struct {
    int     fd;
    int     flags;
    int     error;
    int     eof;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

FILE*   fopen(const char* path, const char* mode);
int     fclose(FILE* stream);
size_t  fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t  fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int     fgetc(FILE* stream);
int     fputc(int c, FILE* stream);
char*   fgets(char* s, int size, FILE* stream);
int     fputs(const char* s, FILE* stream);
int     fprintf(FILE* stream, const char* fmt, ...);

#endif

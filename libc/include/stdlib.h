#ifndef _MONO_STDLIB_H
#define _MONO_STDLIB_H

#include "types.h"

/* Memory allocation */
void*  malloc(size_t size);
void   free(void* ptr);
void*  calloc(size_t nmemb, size_t size);
void*  realloc(void* ptr, size_t size);

/* Process control */
void   exit(int status) __attribute__((noreturn));
void   abort(void) __attribute__((noreturn));

/* String conversion */
int    atoi(const char* str);
long   atol(const char* str);
char*  itoa_buf(int value, char* buf, int base);

/* Pseudo-random numbers */
int    rand(void);
void   srand(unsigned int seed);

/* Environment */
char*  getenv(const char* name);

/* Absolute value */
int    abs(int n);

#endif

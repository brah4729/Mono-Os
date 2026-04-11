#ifndef _MONO_UNISTD_H
#define _MONO_UNISTD_H

#include "types.h"

/* Process management */
void    _exit(int status) __attribute__((noreturn));
pid_t   getpid(void);
pid_t   fork(void);
int     execv(const char* path, char* const argv[]);
pid_t   waitpid(pid_t pid, int* status, int options);

/* File I/O */
int     open(const char* path, int flags);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int     close(int fd);
off_t   lseek(int fd, off_t offset, int whence);
int     unlink(const char* path);
int     symlink(const char* target, const char* linkpath);
ssize_t readlink(const char* path, char* buf, size_t bufsiz);

/* Directory */
int     mkdir(const char* path);
int     chdir(const char* path);
char*   getcwd(char* buf, size_t size);

/* Memory */
void*   sbrk(int increment);

#endif

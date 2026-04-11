/* libc/unistd.c — POSIX-style system call wrappers for MonoOS */

#include "include/unistd.h"
#include "include/syscall.h"

void _exit(int status) {
    _syscall1(_SYS_EXIT, status);
    /* Should never return, but just in case */
    for (;;) __asm__ volatile ("hlt");
}

pid_t getpid(void) {
    return (pid_t)_syscall0(_SYS_GETPID);
}

pid_t fork(void) {
    return (pid_t)_syscall0(_SYS_FORK);
}

int execv(const char* path, char* const argv[]) {
    (void)argv; /* TODO: pass argv through */
    return _syscall1(_SYS_EXEC, (int)path);
}

pid_t waitpid(pid_t pid, int* status, int options) {
    (void)status;
    (void)options;
    return (pid_t)_syscall1(_SYS_WAITPID, (int)pid);
}

int open(const char* path, int flags) {
    return _syscall2(_SYS_OPEN, (int)path, flags);
}

ssize_t read(int fd, void* buf, size_t count) {
    return (ssize_t)_syscall3(_SYS_READ, fd, (int)buf, (int)count);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return (ssize_t)_syscall3(_SYS_WRITE, fd, (int)buf, (int)count);
}

int close(int fd) {
    return _syscall1(_SYS_CLOSE, fd);
}

off_t lseek(int fd, off_t offset, int whence) {
    return (off_t)_syscall3(_SYS_SEEK, fd, (int)offset, whence);
}

int unlink(const char* path) {
    return _syscall1(_SYS_UNLINK, (int)path);
}

int symlink(const char* target, const char* linkpath) {
    return _syscall2(_SYS_SYMLINK, (int)target, (int)linkpath);
}

ssize_t readlink(const char* path, char* buf, size_t bufsiz) {
    return (ssize_t)_syscall3(_SYS_READLINK, (int)path, (int)buf, (int)bufsiz);
}

int mkdir(const char* path) {
    return _syscall1(_SYS_MKDIR, (int)path);
}

int chdir(const char* path) {
    return _syscall1(_SYS_CHDIR, (int)path);
}

char* getcwd(char* buf, size_t size) {
    int ret = _syscall2(_SYS_GETCWD, (int)buf, (int)size);
    return (ret == 0) ? buf : NULL;
}

/* sbrk — expand/shrink the heap using brk syscall */
static uint32_t _current_brk = 0;

void* sbrk(int increment) {
    if (_current_brk == 0) {
        /* Initialize: get current break from kernel */
        _current_brk = (uint32_t)_syscall1(_SYS_BRK, 0);
        if (_current_brk == (uint32_t)-1) return (void*)-1;
    }

    if (increment == 0) return (void*)_current_brk;

    uint32_t old_brk = _current_brk;
    uint32_t new_brk = old_brk + increment;

    int ret = _syscall1(_SYS_BRK, (int)new_brk);
    if (ret == -1) return (void*)-1;

    _current_brk = new_brk;
    return (void*)old_brk;
}

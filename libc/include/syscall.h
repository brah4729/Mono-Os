#ifndef _MONO_SYSCALL_H
#define _MONO_SYSCALL_H

/*
 * monolibc — internal syscall interface
 *
 * Register convention (Linux i386 style):
 *   eax = syscall number
 *   ebx = arg1, ecx = arg2, edx = arg3
 *   esi = arg4, edi = arg5
 *   eax = return value
 */

/* Syscall numbers — must match kernel/include/syscall.h */
#define _SYS_EXIT       0
#define _SYS_READ       1
#define _SYS_WRITE      2
#define _SYS_OPEN       3
#define _SYS_CLOSE      4
#define _SYS_STAT       5
#define _SYS_SEEK       6
#define _SYS_READDIR    7
#define _SYS_EXEC       8
#define _SYS_FORK       9
#define _SYS_WAITPID    10
#define _SYS_BRK        11
#define _SYS_GETPID     12
#define _SYS_MKDIR      13
#define _SYS_UNLINK     14
#define _SYS_SYMLINK    15
#define _SYS_READLINK   16
#define _SYS_GETCWD     17
#define _SYS_CHDIR      18

/* Inline syscall wrappers using int 0x80 */

static inline int _syscall0(int num) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num)
        : "memory"
    );
    return ret;
}

static inline int _syscall1(int num, int arg1) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1)
        : "memory"
    );
    return ret;
}

static inline int _syscall2(int num, int arg1, int arg2) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2)
        : "memory"
    );
    return ret;
}

static inline int _syscall3(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory"
    );
    return ret;
}

#endif

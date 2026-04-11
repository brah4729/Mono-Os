/* userland/hello.c — First userland program for MonoOS */

#include "../libc/include/stdio.h"
#include "../libc/include/unistd.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("Hello from userland!\n");
    printf("PID: %d\n", getpid());
    printf("MonoOS is alive.\n");

    return 0;
}

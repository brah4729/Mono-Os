/* userland/init.c — PID 1 init process for MonoOS
 *
 * This is the first userland process launched by the kernel.
 * It sets up the /nix/store structure and spawns the shell.
 */

#include "../libc/include/stdio.h"
#include "../libc/include/stdlib.h"
#include "../libc/include/unistd.h"
#include "../libc/include/string.h"

/* Ensure essential directories exist */
static void setup_filesystem(void) {
    mkdir("/bin");
    mkdir("/etc");
    mkdir("/tmp");
    mkdir("/var");
    mkdir("/var/log");
    mkdir("/nix");
    mkdir("/nix/store");
    mkdir("/nix/var");
    mkdir("/nix/var/nix");
    mkdir("/nix/var/nix/profiles");
    mkdir("/nix/var/nix/profiles/default");
}

/* Write a simple config file */
static void write_config(void) {
    int fd = open("/etc/hostname", O_WRONLY | O_CREAT | O_TRUNC);
    if (fd >= 0) {
        const char* name = "monoos\n";
        write(fd, name, strlen(name));
        close(fd);
    }

    fd = open("/etc/os-release", O_WRONLY | O_CREAT | O_TRUNC);
    if (fd >= 0) {
        const char* release =
            "NAME=\"MonoOS\"\n"
            "VERSION=\"0.2.0\"\n"
            "ID=monoos\n"
            "PRETTY_NAME=\"MonoOS 0.2.0 (Dori)\"\n"
            "HOME_URL=\"https://monoos.dev\"\n";
        write(fd, release, strlen(release));
        close(fd);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("[init] MonoOS init (PID %d) starting...\n", getpid());

    /* Set up filesystem hierarchy */
    printf("[init] Creating directory structure...\n");
    setup_filesystem();

    /* Write configuration files */
    printf("[init] Writing system configuration...\n");
    write_config();

    printf("[init] System initialization complete.\n");
    printf("[init] MonoOS 0.2.0 (Dori kernel) ready.\n");

    /* In a full OS, we'd spawn a shell here with fork+exec.
     * For now, loop forever keeping PID 1 alive. */
    while (1) {
        /* Reap zombie children (simplified) */
        waitpid(-1, NULL, 0);
    }

    return 0;
}

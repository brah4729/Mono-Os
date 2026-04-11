/* userland/nix-env.c — Nix profile manager for MonoOS
 *
 * Manages Nix-style profiles and package activation.
 * Profiles live in /nix/var/nix/profiles/ and symlink to store paths.
 *
 * Usage:
 *   nix-env -i <store-path>  — Install (activate) a package
 *   nix-env -e <name>        — Remove a package from profile
 *   nix-env -q               — List installed packages in profile
 *   nix-env --rollback       — Rollback to previous generation
 */

#include "../libc/include/stdio.h"
#include "../libc/include/stdlib.h"
#include "../libc/include/unistd.h"
#include "../libc/include/string.h"

#define PROFILE_DIR     "/nix/var/nix/profiles/default"
#define NIX_STORE_PATH  "/nix/store"
#define MAX_PROFILE_ENTRIES 128
#define MAX_PATH_LEN    512

/* Profile entry — maps a name to a store path */
typedef struct {
    char name[256];
    char store_path[MAX_PATH_LEN];
    int  active;
} profile_entry_t;

/* Profile manifest file format:
 *   Each line: <name>\t<store_path>\n
 *   Stored in /nix/var/nix/profiles/default/manifest
 */

#define MANIFEST_PATH PROFILE_DIR "/manifest"

/* Dirent structure matching kernel VFS */
typedef struct {
    char     name[256];
    unsigned int inode;
    unsigned char type;
} mono_dirent_t;

/* Parse a manifest line: "name\tstore_path\n" */
static int parse_manifest_line(const char* line, profile_entry_t* entry) {
    int i = 0;

    /* Read name until tab */
    while (line[i] && line[i] != '\t' && i < 255) {
        entry->name[i] = line[i];
        i++;
    }
    entry->name[i] = '\0';

    if (line[i] != '\t') return -1;
    i++; /* skip tab */

    /* Read store path until newline or end */
    int j = 0;
    while (line[i] && line[i] != '\n' && j < MAX_PATH_LEN - 1) {
        entry->store_path[j++] = line[i++];
    }
    entry->store_path[j] = '\0';

    entry->active = 1;
    return 0;
}

/* Read the manifest file into entries array */
static int read_manifest(profile_entry_t* entries, int max_entries) {
    int fd = open(MANIFEST_PATH, O_RDONLY);
    if (fd < 0) return 0;

    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0) return 0;
    buf[n] = '\0';

    int count = 0;
    char* line = buf;

    while (*line && count < max_entries) {
        /* Find end of line */
        char* eol = strchr(line, '\n');
        if (eol) *eol = '\0';

        if (*line) {
            if (parse_manifest_line(line, &entries[count]) == 0) {
                count++;
            }
        }

        if (eol) line = eol + 1;
        else break;
    }

    return count;
}

/* Write the manifest file from entries array */
static int write_manifest(profile_entry_t* entries, int count) {
    int fd = open(MANIFEST_PATH, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) return -1;

    char line[MAX_PATH_LEN + 256 + 2];
    for (int i = 0; i < count; i++) {
        if (!entries[i].active) continue;

        int len = 0;
        /* name\tstore_path\n */
        len = snprintf(line, sizeof(line), "%s\t%s\n",
                       entries[i].name, entries[i].store_path);
        write(fd, line, len);
    }

    close(fd);
    return 0;
}

/* Install a package: add to profile manifest */
static int cmd_install(const char* store_path) {
    profile_entry_t entries[MAX_PROFILE_ENTRIES];
    int count = read_manifest(entries, MAX_PROFILE_ENTRIES);

    /* Extract package name from store path */
    const char* basename = strrchr(store_path, '/');
    if (basename) basename++;
    else basename = store_path;

    /* Parse hash-name format */
    char pkg_name[256] = {0};
    const char* dash = strchr(basename, '-');
    if (dash) {
        strcpy(pkg_name, dash + 1);
    } else {
        strcpy(pkg_name, basename);
    }

    /* Check if already installed */
    for (int i = 0; i < count; i++) {
        if (entries[i].active && strcmp(entries[i].name, pkg_name) == 0) {
            printf("Package '%s' is already installed.\n", pkg_name);
            printf("  Updating: %s -> %s\n", entries[i].store_path, store_path);
            strcpy(entries[i].store_path, store_path);
            write_manifest(entries, count);
            printf("Updated.\n");
            return 0;
        }
    }

    /* Add new entry */
    if (count >= MAX_PROFILE_ENTRIES) {
        printf("Error: Profile is full (max %d packages).\n", MAX_PROFILE_ENTRIES);
        return 1;
    }

    strcpy(entries[count].name, pkg_name);
    strcpy(entries[count].store_path, store_path);
    entries[count].active = 1;
    count++;

    write_manifest(entries, count);

    printf("Installed: %s\n", pkg_name);
    printf("  Store path: %s\n", store_path);

    /* Create symlink in profile dir */
    char link_path[MAX_PATH_LEN];
    snprintf(link_path, sizeof(link_path), "%s/%s", PROFILE_DIR, pkg_name);
    symlink(store_path, link_path);

    return 0;
}

/* Remove a package from the profile */
static int cmd_remove(const char* pkg_name) {
    profile_entry_t entries[MAX_PROFILE_ENTRIES];
    int count = read_manifest(entries, MAX_PROFILE_ENTRIES);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (entries[i].active && strcmp(entries[i].name, pkg_name) == 0) {
            entries[i].active = 0;
            found = 1;
            printf("Removed: %s\n", pkg_name);
            printf("  (store path %s still exists)\n", entries[i].store_path);
            break;
        }
    }

    if (!found) {
        printf("Package '%s' is not installed.\n", pkg_name);
        return 1;
    }

    write_manifest(entries, count);

    /* Remove symlink */
    char link_path[MAX_PATH_LEN];
    snprintf(link_path, sizeof(link_path), "%s/%s", PROFILE_DIR, pkg_name);
    unlink(link_path);

    return 0;
}

/* List installed packages */
static int cmd_query(void) {
    profile_entry_t entries[MAX_PROFILE_ENTRIES];
    int count = read_manifest(entries, MAX_PROFILE_ENTRIES);

    printf("\033[1;36m");
    printf("Installed packages (profile: default):\n");
    printf("\033[0m");
    printf("%-30s  %s\n", "PACKAGE", "STORE PATH");
    printf("%-30s  %s\n", "-------", "----------");

    int active_count = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].active) {
            printf("%-30s  %s\n", entries[i].name, entries[i].store_path);
            active_count++;
        }
    }

    if (active_count == 0) {
        printf("  (no packages installed)\n");
    } else {
        printf("\n%d package(s) in profile.\n", active_count);
    }

    return 0;
}

/* Rollback — restore previous manifest generation */
static int cmd_rollback(void) {
    /* Check for backup manifest */
    int fd = open(MANIFEST_PATH ".bak", O_RDONLY);
    if (fd < 0) {
        printf("No previous generation to roll back to.\n");
        return 1;
    }

    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0) {
        printf("Previous generation is empty.\n");
        return 1;
    }
    buf[n] = '\0';

    /* Save current as backup of backup (gen-2) */
    /* Then restore backup as current */
    int fd_out = open(MANIFEST_PATH, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd_out < 0) {
        printf("Error: Cannot write manifest.\n");
        return 1;
    }
    write(fd_out, buf, n);
    close(fd_out);

    printf("Rolled back to previous generation.\n");
    return 0;
}

static void usage(void) {
    printf("nix-env — MonoOS package profile manager\n\n");
    printf("Usage:\n");
    printf("  nix-env -i PATH    Install package from store path\n");
    printf("  nix-env -e NAME    Remove package from profile\n");
    printf("  nix-env -q         List installed packages\n");
    printf("  nix-env --rollback Rollback to previous generation\n");
    printf("  nix-env -h         Show this help\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }

    if (strcmp(argv[1], "-q") == 0 || strcmp(argv[1], "--query") == 0) {
        return cmd_query();
    }

    if (strcmp(argv[1], "--rollback") == 0) {
        return cmd_rollback();
    }

    if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--install") == 0) {
        if (argc < 3) {
            printf("Error: -i requires a store path argument.\n");
            return 1;
        }
        return cmd_install(argv[2]);
    }

    if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--remove") == 0) {
        if (argc < 3) {
            printf("Error: -e requires a package name.\n");
            return 1;
        }
        return cmd_remove(argv[2]);
    }

    printf("Unknown option: %s\n", argv[1]);
    usage();
    return 1;
}

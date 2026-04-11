/* libc/stdio.c — Standard I/O for monolibc */

#include "include/stdio.h"
#include "include/unistd.h"
#include "include/string.h"
#include "include/stdlib.h"

/* Standard streams */
static FILE _stdin  = { .fd = STDIN_FILENO,  .flags = O_RDONLY, .error = 0, .eof = 0 };
static FILE _stdout = { .fd = STDOUT_FILENO, .flags = O_WRONLY, .error = 0, .eof = 0 };
static FILE _stderr = { .fd = STDERR_FILENO, .flags = O_WRONLY, .error = 0, .eof = 0 };

FILE* stdin  = &_stdin;
FILE* stdout = &_stdout;
FILE* stderr = &_stderr;

/* ─── Basic I/O ─────────────────────────────────────────── */

int putchar(int c) {
    char ch = (char)c;
    write(STDOUT_FILENO, &ch, 1);
    return c;
}

int puts(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}

int getchar(void) {
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return EOF;
    return (int)(unsigned char)c;
}

/* ─── String formatting (snprintf engine) ────────────────── */

/* Internal: write a single char to buffer if space permits */
static int _buf_putchar(char* buf, size_t pos, size_t size, char c) {
    if (pos < size - 1) {
        buf[pos] = c;
    }
    return 1;
}

/* Internal: write a string to buffer */
static int _buf_puts(char* buf, size_t pos, size_t size, const char* str) {
    int written = 0;
    while (*str) {
        written += _buf_putchar(buf, pos + written, size, *str++);
    }
    return written;
}

/* Internal: convert unsigned int to string */
static int _buf_put_uint(char* buf, size_t pos, size_t size, unsigned int val, int base, int width, char pad) {
    char tmp[32];
    int i = 0;

    if (val == 0) {
        tmp[i++] = '0';
    } else {
        while (val > 0) {
            int digit = val % base;
            tmp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            val /= base;
        }
    }

    /* Pad */
    int written = 0;
    while (i + written < width) {
        written += _buf_putchar(buf, pos + written, size, pad);
    }

    /* Reverse output */
    while (i > 0) {
        written += _buf_putchar(buf, pos + written, size, tmp[--i]);
    }

    return written;
}

/* Internal: convert signed int to string */
static int _buf_put_int(char* buf, size_t pos, size_t size, int val, int width, char pad) {
    int written = 0;

    if (val < 0) {
        written += _buf_putchar(buf, pos + written, size, '-');
        val = -val;
        if (width > 0) width--;
    }

    written += _buf_put_uint(buf, pos + written, size, (unsigned int)val, 10, width, pad);
    return written;
}

int snprintf(char* buf, size_t size, const char* fmt, ...) {
    /* Use GCC built-in va_list */
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    size_t pos = 0;

    while (*fmt) {
        if (*fmt != '%') {
            pos += _buf_putchar(buf, pos, size, *fmt++);
            continue;
        }

        fmt++; /* skip '%' */

        /* Parse flags */
        char pad = ' ';
        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }

        /* Parse width */
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        /* Parse conversion */
        switch (*fmt) {
            case 'd':
            case 'i': {
                int val = __builtin_va_arg(args, int);
                pos += _buf_put_int(buf, pos, size, val, width, pad);
                break;
            }
            case 'u': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                pos += _buf_put_uint(buf, pos, size, val, 10, width, pad);
                break;
            }
            case 'x': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                pos += _buf_put_uint(buf, pos, size, val, 16, width, pad);
                break;
            }
            case 'p': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                pos += _buf_puts(buf, pos, size, "0x");
                pos += _buf_put_uint(buf, pos, size, val, 16, 8, '0');
                break;
            }
            case 's': {
                const char* str = __builtin_va_arg(args, const char*);
                if (!str) str = "(null)";
                int len = (int)strlen(str);
                /* Right-pad with spaces if width > len */
                pos += _buf_puts(buf, pos, size, str);
                while (len < width) {
                    pos += _buf_putchar(buf, pos, size, ' ');
                    len++;
                }
                break;
            }
            case 'c': {
                char c = (char)__builtin_va_arg(args, int);
                pos += _buf_putchar(buf, pos, size, c);
                break;
            }
            case '%':
                pos += _buf_putchar(buf, pos, size, '%');
                break;
            default:
                pos += _buf_putchar(buf, pos, size, '%');
                pos += _buf_putchar(buf, pos, size, *fmt);
                break;
        }
        fmt++;
    }

    /* Null-terminate */
    if (size > 0) {
        buf[pos < size ? pos : size - 1] = '\0';
    }

    __builtin_va_end(args);
    return (int)pos;
}

int printf(const char* fmt, ...) {
    char buf[512];

    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    /* Reimplement by calling snprintf logic manually.
     * Since snprintf uses va_list, and we can't forward va_list easily
     * in C89, we use a simpler approach: format to buffer then write. */

    /* We need to reconstruct the va_list call... let's use a helper */
    __builtin_va_end(args);

    /* Fallback: use snprintf with raw args reparse */
    /* This is a simplified printf that formats directly */
    __builtin_va_start(args, fmt);

    size_t pos = 0;
    size_t size = sizeof(buf);

    while (*fmt && pos < size - 1) {
        if (*fmt != '%') {
            buf[pos++] = *fmt++;
            continue;
        }

        fmt++;

        char pad = ' ';
        if (*fmt == '0') { pad = '0'; fmt++; }

        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        switch (*fmt) {
            case 'd': case 'i': {
                int val = __builtin_va_arg(args, int);
                pos += _buf_put_int(buf, pos, size, val, width, pad);
                break;
            }
            case 'u': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                pos += _buf_put_uint(buf, pos, size, val, 10, width, pad);
                break;
            }
            case 'x': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                pos += _buf_put_uint(buf, pos, size, val, 16, width, pad);
                break;
            }
            case 'p': {
                unsigned int val = __builtin_va_arg(args, unsigned int);
                pos += _buf_puts(buf, pos, size, "0x");
                pos += _buf_put_uint(buf, pos, size, val, 16, 8, '0');
                break;
            }
            case 's': {
                const char* str = __builtin_va_arg(args, const char*);
                if (!str) str = "(null)";
                pos += _buf_puts(buf, pos, size, str);
                break;
            }
            case 'c': {
                char c = (char)__builtin_va_arg(args, int);
                buf[pos++] = c;
                break;
            }
            case '%':
                buf[pos++] = '%';
                break;
            default:
                buf[pos++] = '%';
                if (pos < size - 1) buf[pos++] = *fmt;
                break;
        }
        fmt++;
    }

    buf[pos] = '\0';
    __builtin_va_end(args);

    return (int)write(STDOUT_FILENO, buf, pos);
}

/* ─── FILE-based I/O ─────────────────────────────────────── */

FILE* fopen(const char* path, const char* mode) {
    int flags = 0;

    if (strcmp(mode, "r") == 0)       flags = O_RDONLY;
    else if (strcmp(mode, "w") == 0)  flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (strcmp(mode, "a") == 0)  flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (strcmp(mode, "r+") == 0) flags = O_RDWR;
    else if (strcmp(mode, "w+") == 0) flags = O_RDWR | O_CREAT | O_TRUNC;
    else if (strcmp(mode, "a+") == 0) flags = O_RDWR | O_CREAT | O_APPEND;
    else return NULL;

    int fd = open(path, flags);
    if (fd < 0) return NULL;

    /* Allocate FILE struct */
    FILE* f = (FILE*)malloc(sizeof(FILE));
    if (!f) {
        close(fd);
        return NULL;
    }

    f->fd = fd;
    f->flags = flags;
    f->error = 0;
    f->eof = 0;
    return f;
}

int fclose(FILE* stream) {
    if (!stream) return EOF;
    int ret = close(stream->fd);
    if (stream != stdin && stream != stdout && stream != stderr) {
        free(stream);
    }
    return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream || size == 0 || nmemb == 0) return 0;

    size_t total = size * nmemb;
    ssize_t n = read(stream->fd, ptr, total);

    if (n < 0) { stream->error = 1; return 0; }
    if (n == 0) { stream->eof = 1; return 0; }

    return (size_t)n / size;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream || size == 0 || nmemb == 0) return 0;

    size_t total = size * nmemb;
    ssize_t n = write(stream->fd, ptr, total);

    if (n < 0) { stream->error = 1; return 0; }
    return (size_t)n / size;
}

int fgetc(FILE* stream) {
    char c;
    ssize_t n = read(stream->fd, &c, 1);
    if (n <= 0) {
        stream->eof = 1;
        return EOF;
    }
    return (int)(unsigned char)c;
}

int fputc(int c, FILE* stream) {
    char ch = (char)c;
    ssize_t n = write(stream->fd, &ch, 1);
    if (n <= 0) { stream->error = 1; return EOF; }
    return c;
}

char* fgets(char* s, int size, FILE* stream) {
    if (!s || size <= 0 || !stream) return NULL;

    int i = 0;
    while (i < size - 1) {
        int c = fgetc(stream);
        if (c == EOF) {
            if (i == 0) return NULL;
            break;
        }
        s[i++] = (char)c;
        if (c == '\n') break;
    }
    s[i] = '\0';
    return s;
}

int fputs(const char* s, FILE* stream) {
    size_t len = strlen(s);
    ssize_t n = write(stream->fd, s, len);
    if (n < 0) return EOF;
    return (int)n;
}

int fprintf(FILE* stream, const char* fmt, ...) {
    char buf[512];

    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    /* Simple: format to buffer, write to stream */
    size_t pos = 0;
    size_t bsize = sizeof(buf);

    while (*fmt && pos < bsize - 1) {
        if (*fmt != '%') {
            buf[pos++] = *fmt++;
            continue;
        }
        fmt++;

        char pad = ' ';
        if (*fmt == '0') { pad = '0'; fmt++; }

        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        switch (*fmt) {
            case 'd': case 'i':
                pos += _buf_put_int(buf, pos, bsize,
                    __builtin_va_arg(args, int), width, pad);
                break;
            case 'u':
                pos += _buf_put_uint(buf, pos, bsize,
                    __builtin_va_arg(args, unsigned int), 10, width, pad);
                break;
            case 'x':
                pos += _buf_put_uint(buf, pos, bsize,
                    __builtin_va_arg(args, unsigned int), 16, width, pad);
                break;
            case 's': {
                const char* str = __builtin_va_arg(args, const char*);
                if (!str) str = "(null)";
                pos += _buf_puts(buf, pos, bsize, str);
                break;
            }
            case 'c':
                buf[pos++] = (char)__builtin_va_arg(args, int);
                break;
            case '%':
                buf[pos++] = '%';
                break;
            default:
                buf[pos++] = '%';
                if (pos < bsize - 1) buf[pos++] = *fmt;
                break;
        }
        fmt++;
    }

    buf[pos] = '\0';
    __builtin_va_end(args);

    return (int)write(stream->fd, buf, pos);
}

/* libc/stdlib.c — Standard library functions for monolibc */

#include "include/stdlib.h"
#include "include/unistd.h"
#include "include/string.h"

/* ─── Process control ──────────────────────────────────── */

void exit(int status) {
    _exit(status);
}

void abort(void) {
    _exit(-1);
}

/* ─── String conversion ─────────────────────────────────── */

int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    /* Skip whitespace */
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;

    /* Handle sign */
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') { str++; }

    /* Convert digits */
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

long atol(const char* str) {
    return (long)atoi(str);
}

char* itoa_buf(int value, char* buf, int base) {
    char* p = buf;
    char* p1;
    char* p2;
    unsigned int ud;
    int negative = 0;

    if (base == 10 && value < 0) {
        negative = 1;
        ud = (unsigned int)(-(value + 1)) + 1;
    } else {
        ud = (unsigned int)value;
    }

    do {
        int remainder = ud % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= base);

    if (negative) *p++ = '-';
    *p = '\0';

    /* Reverse */
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }

    return buf;
}

/* ─── Memory allocation ─────────────────────────────────── */

/*
 * Simple first-fit allocator using sbrk().
 * Header stores block size and free flag.
 */

typedef struct heap_block {
    size_t size;
    int    free;
    struct heap_block* next;
} heap_block_t;

#define BLOCK_HEADER_SIZE sizeof(heap_block_t)
#define ALIGN4(x) (((x) + 3) & ~3)

static heap_block_t* heap_head = NULL;

/* Find a free block that fits */
static heap_block_t* find_free_block(size_t size) {
    heap_block_t* block = heap_head;
    while (block) {
        if (block->free && block->size >= size) {
            return block;
        }
        block = block->next;
    }
    return NULL;
}

/* Request more memory from OS */
static heap_block_t* request_space(size_t size) {
    heap_block_t* block = (heap_block_t*)sbrk(0);
    void* request = sbrk((int)(BLOCK_HEADER_SIZE + size));
    if (request == (void*)-1) return NULL;

    block->size = size;
    block->free = 0;
    block->next = NULL;

    if (heap_head) {
        /* Find the last block and link */
        heap_block_t* last = heap_head;
        while (last->next) last = last->next;
        last->next = block;
    } else {
        heap_head = block;
    }

    return block;
}

void* malloc(size_t size) {
    if (size == 0) return NULL;

    size = ALIGN4(size);

    /* Try to find a free block */
    heap_block_t* block = find_free_block(size);
    if (block) {
        block->free = 0;
        return (void*)(block + 1);
    }

    /* Request new space */
    block = request_space(size);
    if (!block) return NULL;

    return (void*)(block + 1);
}

void free(void* ptr) {
    if (!ptr) return;

    heap_block_t* block = (heap_block_t*)ptr - 1;
    block->free = 1;

    /* Coalesce adjacent free blocks */
    heap_block_t* current = heap_head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += BLOCK_HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* ptr = malloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return NULL; }

    heap_block_t* block = (heap_block_t*)ptr - 1;
    if (block->size >= size) return ptr;

    void* new_ptr = malloc(size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size);
    free(ptr);
    return new_ptr;
}

/* ─── Pseudo-random numbers ──────────────────────────────── */

static unsigned int _rand_seed = 1;

int rand(void) {
    _rand_seed = _rand_seed * 1103515245 + 12345;
    return (int)((_rand_seed / 65536) % 32768);
}

void srand(unsigned int seed) {
    _rand_seed = seed;
}

/* ─── Misc ───────────────────────────────────────────────── */

char* getenv(const char* name) {
    (void)name;
    return NULL;  /* No environment variables yet */
}

int abs(int n) {
    return (n < 0) ? -n : n;
}

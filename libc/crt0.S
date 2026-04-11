; libc/crt0.S — C runtime startup for MonoOS userland programs
;
; This is the entry point for every userland executable.
; The kernel's ELF loader jumps here. We set up the stack,
; call main(), then invoke sys_exit() with main's return value.

[BITS 32]
[GLOBAL _start]
[EXTERN main]

section .text

_start:
    ; The kernel has already set up our stack pointer.
    ; Clear the base pointer for clean stack traces
    xor ebp, ebp

    ; Call main()
    ; TODO: pass argc, argv when we have command-line argument support
    push 0          ; argv = NULL
    push 0          ; argc = 0
    call main
    add esp, 8      ; Clean up argc/argv

    ; main() returned — call exit(return_value)
    ; eax holds main's return value
    mov ebx, eax    ; arg1 = exit code
    mov eax, 0      ; SYS_EXIT = 0
    int 0x80        ; syscall

    ; Should never reach here
.hang:
    hlt
    jmp .hang

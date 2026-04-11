/* Linker script for MonoOS userland programs */
/* Places user programs at 0x400000 (4 MiB) */

ENTRY(_start)

SECTIONS
{
    . = 0x400000;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    _end = .;
}

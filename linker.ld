/* linker.ld — tells the linker where to place each section in memory */

ENTRY(_start)               /* Entry point symbol from boot.asm */

SECTIONS {
    /* Load the kernel at 1MB — conventional for x86 kernels */
    . = 1M;

    /* Multiboot header MUST be in the first 32KB of the file */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }

    /* Executable code */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Read-only data (string literals, const globals) */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Initialized writable data */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Uninitialized data — zeroed by convention */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
}

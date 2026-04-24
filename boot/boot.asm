; boot.asm - Multiboot2 entry point
; This is the FIRST code that runs when GRUB hands control to our kernel

bits 32                     ; We start in 32-bit protected mode

; Multiboot2 header - GRUB looks for this magic signature
section .multiboot
align 8
mb_header_start:
    dd 0xE85250D6           ; Multiboot2 magic number
    dd 0                    ; Architecture: i386 protected mode
    dd mb_header_end - mb_header_start  ; Header length
    dd -(0xE85250D6 + 0 + (mb_header_end - mb_header_start))  ; Checksum

    ; End tag
    dw 0                    ; Type 0 = end
    dw 0                    ; Flags
    dd 8                    ; Size
mb_header_end:

; BSS section - uninitialized data (our stack lives here)
section .bss
align 16
stack_bottom:
    resb 16384              ; Reserve 16 KB for the stack
stack_top:

; Text section - our actual code
section .text
global _start               ; Export _start so the linker can find it

_start:
    ; Set up our stack pointer
    mov esp, stack_top

    ; Call our C kernel main function
    extern kernel_main
    call kernel_main

    ; If kernel_main ever returns, hang the CPU
.hang:
    cli                     ; Disable interrupts
    hlt                     ; Halt
    jmp .hang               ; Loop forever just in case

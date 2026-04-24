; gdt_asm.asm — loads a new GDT and reloads segment registers
bits 32

global gdt_flush

gdt_flush:
    mov eax, [esp+4]    ; Get the pointer to our gdt_ptr_t struct
    lgdt [eax]          ; Load it into the GDTR register

    ; Reload all data segment registers with the new kernel data selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS with the kernel code selector (0x08)
    ; This is the only way to reload CS in x86
    jmp 0x08:.flush
.flush:
    ret

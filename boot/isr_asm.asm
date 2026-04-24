; isr_asm.asm — ISR and IRQ stubs
; Each stub saves CPU state, calls a C handler, then restores state.
bits 32

; ---------------------------------------------------------------
; Macro: ISR without an error code (CPU doesn't push one)
; We push a dummy 0 so the stack frame is always the same shape.
; ---------------------------------------------------------------
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0         ; dummy error code
    push byte %1        ; interrupt number
    jmp isr_common
%endmacro

; ---------------------------------------------------------------
; Macro: ISR WITH an error code (CPU pushes it automatically)
; ---------------------------------------------------------------
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1        ; interrupt number (error code already on stack)
    jmp isr_common
%endmacro

; ---------------------------------------------------------------
; Macro: Hardware IRQ stub
; IRQs are remapped to interrupts 32-47.
; ---------------------------------------------------------------
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0         ; no error code
    push byte %2        ; interrupt number (32 + irq number)
    jmp irq_common
%endmacro

; CPU Exception stubs (0-15)
ISR_NOERRCODE 0     ; Division by zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non-maskable interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound range exceeded
ISR_NOERRCODE 6     ; Invalid opcode
ISR_NOERRCODE 7     ; Device not available
ISR_ERRCODE   8     ; Double fault (has error code)
ISR_NOERRCODE 9     ; Coprocessor segment overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment not present
ISR_ERRCODE   12    ; Stack-segment fault
ISR_ERRCODE   13    ; General protection fault
ISR_ERRCODE   14    ; Page fault
ISR_NOERRCODE 15    ; Reserved

; Hardware IRQ stubs (mapped to interrupt vectors 32-47)
IRQ  0, 32   ; Programmable Interval Timer
IRQ  1, 33   ; PS/2 Keyboard
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; ---------------------------------------------------------------
; Common ISR handler — save state, call C, restore state
; ---------------------------------------------------------------
extern isr_handler

isr_common:
    pusha               ; Push eax,ecx,edx,ebx,esp,ebp,esi,edi
    mov ax, ds
    push eax            ; Save data segment

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler    ; Call C handler with pointer to registers_t on stack

    pop eax             ; Restore original data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore general purpose registers
    add esp, 8          ; Pop int_no and err_code
    sti
    iret                ; Return from interrupt (restores eip, cs, eflags)

; ---------------------------------------------------------------
; Common IRQ handler — like ISR but sends EOI to PIC
; ---------------------------------------------------------------
extern irq_handler

irq_common:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    sti
    iret

; ---------------------------------------------------------------
; Load the IDT register
; ---------------------------------------------------------------
global idt_flush
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// One 8-byte IDT entry (interrupt gate descriptor)
typedef struct __attribute__((packed)) {
    uint16_t base_low;   // Low 16 bits of handler address
    uint16_t selector;   // Kernel code segment selector (0x08)
    uint8_t  zero;       // Always 0
    uint8_t  flags;      // Gate type + DPL + present bit
    uint16_t base_high;  // High 16 bits of handler address
} idt_entry_t;

// The 6-byte value loaded into the IDTR register
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

// CPU register snapshot saved by our interrupt stubs
typedef struct __attribute__((packed)) {
    uint32_t ds;                            // Data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Pushed by pusha
    uint32_t int_no, err_code;              // Interrupt number + error code
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by CPU automatically
} registers_t;

void idt_init(void);

#endif

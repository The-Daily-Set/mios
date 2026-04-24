#include "idt.h"
#include "vga.h"
#include "keyboard.h"
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Human-readable names for CPU exceptions
static const char* exception_names[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
};

// Called from isr_common in isr_asm.asm
void isr_handler(registers_t* regs) {
    vga_set_color(VGA_WHITE, VGA_RED);
    vga_puts("\n*** KERNEL EXCEPTION ***\n");
    if (regs->int_no < 16) {
        vga_puts(exception_names[regs->int_no]);
    }
    vga_puts("\nSystem halted.\n");
    // Hang — a real OS would try to recover or dump a stack trace
    while (1) { __asm__ volatile ("hlt"); }
}

// IRQ handler dispatch table — install handlers with irq_register()
static void (*irq_handlers[16])(registers_t*) = {0};

void irq_register(uint8_t irq, void (*handler)(registers_t*)) {
    irq_handlers[irq] = handler;
}

// Called from irq_common in isr_asm.asm
void irq_handler(registers_t* regs) {
    uint8_t irq = regs->int_no - 32;

    // Send End-Of-Interrupt to the PIC(s)
    if (irq >= 8) outb(0xA0, 0x20);  // Slave PIC
    outb(0x20, 0x20);                  // Master PIC

    // Dispatch to registered handler if one exists
    if (irq_handlers[irq]) {
        irq_handlers[irq](regs);
    }
}

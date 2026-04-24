#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "shell.h"

void kernel_main(void) {
    vga_init();

    // Boot banner
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("===========================================\n");
    vga_puts("           MyOS v0.2  --  booting         \n");
    vga_puts("===========================================\n\n");

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);

    vga_puts("[  ] Loading GDT...");
    gdt_init();
    vga_puts("\r[OK]\n");

    vga_puts("[  ] Loading IDT...");
    idt_init();
    vga_puts("\r[OK]\n");

    vga_puts("[  ] Initializing keyboard...");
    keyboard_init();
    vga_puts("\r[OK]\n");

    // Enable hardware interrupts
    __asm__ volatile ("sti");
    vga_puts("[OK] Interrupts enabled\n");

    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("\nAll systems go.\n");

    shell_run();

    while (1) { __asm__ volatile ("hlt"); }
}

#include "vga.h"
#include <stdint.h>
#include <stddef.h>

// VGA text buffer is always at this physical address
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)

// I/O port helpers for cursor control
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Terminal state
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color;
static uint16_t* vga_buf;

// Pack a foreground + background color into one VGA attribute byte
static uint8_t make_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

// Pack a character + color attribute into one 16-bit VGA cell
static uint16_t make_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

// Move the hardware cursor to match our tracked position
static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 14);            // High byte register
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);            // Low byte register
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

void vga_init(void) {
    vga_buf = VGA_MEMORY;
    current_color = make_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    current_color = make_color(fg, bg);
}

void vga_clear(void) {
    uint16_t blank = make_entry(' ', current_color);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buf[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}

// Scroll the screen up one line
static void scroll(void) {
    // Move every row up by one
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buf[y * VGA_WIDTH + x] = vga_buf[(y + 1) * VGA_WIDTH + x];
        }
    }
    // Clear the last row
    uint16_t blank = make_entry(' ', current_color);
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buf[cursor_y * VGA_WIDTH + cursor_x] = make_entry(' ', current_color);
        }
    } else if (c == '\t') {
        // Advance to next 4-column tab stop
        cursor_x = (cursor_x + 4) & ~3;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    } else {
        vga_buf[cursor_y * VGA_WIDTH + cursor_x] = make_entry(c, current_color);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= VGA_HEIGHT) {
        scroll();
    }
    update_cursor();
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

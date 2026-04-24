#include "serial.h"
#include <stdint.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);   // Disable interrupts
    outb(COM1 + 3, 0x80);   // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);   // Baud 38400 (divisor low byte)
    outb(COM1 + 1, 0x00);   // Baud 38400 (divisor high byte)
    outb(COM1 + 3, 0x03);   // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);   // Enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B);   // IRQs enabled, RTS/DSR set
}

static int serial_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
    while (!serial_transmit_empty());
    if (c == '\n') {
        outb(COM1, '\r');   // Serial needs CR+LF
        while (!serial_transmit_empty());
    }
    outb(COM1, (uint8_t)c);
}

void serial_puts(const char* s) {
    while (*s) serial_putchar(*s++);
}

#include "keyboard.h"
#include "vga.h"
#include "serial.h"
#include "idt.h"
#include <stdint.h>

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64
#define KB_BUF_SIZE     256

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// Check if serial has a byte waiting (for QEMU nographic input)
static inline int serial_has_data(void) {
    return inb(0x3F8 + 5) & 1;
}
static inline char serial_read_byte(void) {
    return (char)inb(0x3F8);
}

// ---------------------------------------------------------------
// US QWERTY scancode set 1 — maps scan code to ASCII
// Index = scan code, value = ASCII character
// Upper 128: same keys with Shift held
// ---------------------------------------------------------------
static const char scancode_map[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', 0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // F1-F10
    0, 0,                             // NumLock, ScrollLock
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
    0, 0, 0,
    0, 0                              // F11, F12
};

static const char scancode_map_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
    0, 0, 0,
    0, 0
};

// Circular ring buffer for keyboard input
static char kb_buf[KB_BUF_SIZE];
static int  kb_head = 0;
static int  kb_tail = 0;

static int  shift_held   = 0;
static int  caps_lock    = 0;

static void kb_buf_push(char c) {
    int next = (kb_head + 1) % KB_BUF_SIZE;
    if (next != kb_tail) {   // Only push if buffer isn't full
        kb_buf[kb_head] = c;
        kb_head = next;
    }
}

static char kb_buf_pop(void) {
    if (kb_head == kb_tail) return 0;
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return c;
}

// IRQ1 handler — called on every keypress/release
static void keyboard_irq_handler(registers_t* regs) {
    (void)regs;
    uint8_t scancode = inb(KB_DATA_PORT);

    // Key release events have bit 7 set
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_held = 0;
        return;
    }

    // Track modifier keys
    if (scancode == 0x2A || scancode == 0x36) { shift_held = 1; return; }
    if (scancode == 0x3A) { caps_lock = !caps_lock; return; }  // Caps Lock toggle

    if (scancode >= 128) return;

    char c;
    if (shift_held) {
        c = scancode_map_shift[scancode];
    } else {
        c = scancode_map[scancode];
    }

    // Apply caps lock to letters only
    if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
    else if (caps_lock && c >= 'A' && c <= 'Z') c += 32;

    if (c) kb_buf_push(c);
}

void keyboard_init(void) {
    irq_register(1, keyboard_irq_handler);
}

char keyboard_getchar(void) {
    return kb_buf_pop();
}

char keyboard_read(void) {
    while (1) {
        // Check PS/2 keyboard buffer first
        char c = keyboard_getchar();
        if (c) return c;
        // Also check serial port — this is how input arrives in QEMU nographic
        if (serial_has_data()) {
            c = serial_read_byte();
            if (c == '\r') c = '\n';   // Serial sends CR, we want LF
            return c;
        }
        __asm__ volatile ("hlt");
    }
}

void keyboard_readline(char* buf, int len) {
    int i = 0;
    while (i < len - 1) {
        char c = keyboard_read();
        if (c == '\n') {
            vga_putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putchar('\b');
            }
        } else {
            buf[i++] = c;
            vga_putchar(c);
        }
    }
    buf[i] = '\0';
}

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "idt.h"

void keyboard_init(void);

// Returns the next character from the keyboard buffer, or 0 if empty
char keyboard_getchar(void);

// Block until a key is pressed and return it
char keyboard_read(void);

// Read a full line of input into buf (up to len-1 chars), null-terminated
// Supports backspace and echoes typed characters
void keyboard_readline(char* buf, int len);

#endif

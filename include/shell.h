#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

extern volatile uint32_t system_ticks;

void shell_run(void);

#endif

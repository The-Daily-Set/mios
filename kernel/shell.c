#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include <stddef.h>

// ---------------------------------------------------------------
// Tiny string helpers (no stdlib in kernel!)
// ---------------------------------------------------------------
static int k_strlen(const char* s) {
    int n = 0; while (s[n]) n++; return n;
}

static int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *a - *b;
}

static int k_strncmp(const char* a, const char* b, int n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    return n < 0 ? 0 : *a - *b;
}

static void k_strcpy(char* dst, const char* src) {
    while ((*dst++ = *src++));
}

// Split cmd into argv[], returns argc
static int parse_args(char* cmd, char* argv[], int max_args) {
    int argc = 0;
    char* p = cmd;
    while (*p && argc < max_args) {
        while (*p == ' ') p++;   // skip spaces
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) *p++ = '\0';
    }
    return argc;
}

// ---------------------------------------------------------------
// Built-in command implementations
// ---------------------------------------------------------------

static void cmd_help(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("MyOS built-in commands:\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  help       Show this help message\n");
    vga_puts("  clear      Clear the screen\n");
    vga_puts("  echo       Print arguments to screen\n");
    vga_puts("  uname      Show OS information\n");
    vga_puts("  color      Change terminal color (e.g. color green)\n");
    vga_puts("  whoami     Print current user\n");
    vga_puts("  uptime     Show system uptime (ticks)\n");
    vga_puts("  reboot     Reboot the system\n");
    vga_puts("  halt       Halt the CPU\n");
    vga_puts("\nMore commands coming: ls, cat, mkdir, cd, pwd, ps ...\n");
}

static void cmd_uname(int argc, char* argv[]) {
    int all = (argc > 1 && k_strcmp(argv[1], "-a") == 0);
    if (all) {
        vga_puts("MyOS 0.1 myos i686 Beck's Kernel\n");
    } else {
        vga_puts("MyOS\n");
    }
}

static void cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) vga_putchar(' ');
        vga_puts(argv[i]);
    }
    vga_putchar('\n');
}

static void cmd_color(int argc, char* argv[]) {
    if (argc < 2) {
        vga_puts("Usage: color <name>\n");
        vga_puts("Colors: black red green yellow blue magenta cyan white\n");
        return;
    }
    vga_color_t fg = VGA_LIGHT_GREY;
    if      (k_strcmp(argv[1], "red")     == 0) fg = VGA_LIGHT_RED;
    else if (k_strcmp(argv[1], "green")   == 0) fg = VGA_LIGHT_GREEN;
    else if (k_strcmp(argv[1], "yellow")  == 0) fg = VGA_YELLOW;
    else if (k_strcmp(argv[1], "blue")    == 0) fg = VGA_LIGHT_BLUE;
    else if (k_strcmp(argv[1], "magenta") == 0) fg = VGA_LIGHT_MAGENTA;
    else if (k_strcmp(argv[1], "cyan")    == 0) fg = VGA_LIGHT_CYAN;
    else if (k_strcmp(argv[1], "white")   == 0) fg = VGA_WHITE;
    else if (k_strcmp(argv[1], "black")   == 0) fg = VGA_DARK_GREY;
    else { vga_puts("Unknown color.\n"); return; }
    vga_set_color(fg, VGA_BLACK);
    vga_puts("Color changed.\n");
}

// Simple tick counter incremented by the timer IRQ
volatile uint32_t system_ticks = 0;

static void cmd_uptime(void) {
    // Ticks / 100 = seconds (assuming ~100Hz timer, approx)
    uint32_t secs = system_ticks / 100;
    uint32_t mins = secs / 60;
    secs %= 60;
    vga_puts("Uptime: ");
    // Print minutes
    if (mins > 0) {
        char tmp[12];
        int i = 0;
        uint32_t m = mins;
        if (m == 0) { tmp[i++] = '0'; }
        else { int start = i; while (m) { tmp[i++] = '0' + (m % 10); m /= 10; }
               // reverse
               for (int l = start, r = i-1; l < r; l++, r--) {
                   char t = tmp[l]; tmp[l] = tmp[r]; tmp[r] = t;
               }
        }
        tmp[i] = '\0';
        vga_puts(tmp);
        vga_puts("m ");
    }
    // Print seconds
    {
        char tmp[12];
        int i = 0;
        uint32_t s = secs;
        if (s == 0) { tmp[i++] = '0'; }
        else { int start = i; while (s) { tmp[i++] = '0' + (s % 10); s /= 10; }
               for (int l = start, r = i-1; l < r; l++, r--) {
                   char t = tmp[l]; tmp[l] = tmp[r]; tmp[r] = t;
               }
        }
        tmp[i] = '\0';
        vga_puts(tmp);
    }
    vga_puts("s\n");
}

static void cmd_reboot(void) {
    vga_puts("Rebooting...\n");
    // Triple fault method — works in QEMU, most real hardware too
    __asm__ volatile (
        "cli\n"
        "mov $0, %eax\n"
        "mov %eax, %cr3\n"  // Invalidate page tables -> triple fault -> reset
    );
}

static void cmd_halt(void) {
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_puts("System halted. Goodbye.\n");
    __asm__ volatile ("cli; hlt");
}

// ---------------------------------------------------------------
// Shell main loop
// ---------------------------------------------------------------
void shell_run(void) {
    char input[256];
    char* argv[16];

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("\nMyOS shell ready. Type 'help' for commands.\n\n");

    while (1) {
        // Print prompt
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        vga_puts("root@myos");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        vga_puts(":~$ ");
        vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

        // Read a line of input
        keyboard_readline(input, sizeof(input));

        if (input[0] == '\0') continue;

        int argc = parse_args(input, argv, 16);
        if (argc == 0) continue;

        // Dispatch commands
        if      (k_strcmp(argv[0], "help")   == 0) cmd_help();
        else if (k_strcmp(argv[0], "clear")  == 0) vga_clear();
        else if (k_strcmp(argv[0], "echo")   == 0) cmd_echo(argc, argv);
        else if (k_strcmp(argv[0], "uname")  == 0) cmd_uname(argc, argv);
        else if (k_strcmp(argv[0], "color")  == 0) cmd_color(argc, argv);
        else if (k_strcmp(argv[0], "whoami") == 0) vga_puts("root\n");
        else if (k_strcmp(argv[0], "uptime") == 0) cmd_uptime();
        else if (k_strcmp(argv[0], "reboot") == 0) cmd_reboot();
        else if (k_strcmp(argv[0], "halt")   == 0) cmd_halt();
        else {
            vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            vga_puts(argv[0]);
            vga_puts(": command not found\n");
            vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
        }
    }
}

# MyOS

A hobby x86 operating system built from scratch in C and Assembly.

## ▶️ Run in your browser (no installs needed)

1. Push this repo to GitHub (free account is fine)
2. Click the green **Code** button → **Codespaces** → **Create codespace on main**
3. Wait ~2 minutes for tools to install
4. A terminal will open and MyOS will boot automatically

You'll see:
```
==========================================
          MyOS v0.2  --  booting
==========================================

[OK] Loading GDT...
[OK] Loading IDT...
[OK] Initializing keyboard...
[OK] Interrupts enabled

All systems go.

MyOS shell ready. Type 'help' for commands.

root@myos:~$
```

> **To exit QEMU:** Press `Ctrl+A` then `X`

---

## Shell commands

| Command | Description |
|---------|-------------|
| `help` | List all commands |
| `clear` | Clear the screen |
| `echo <text>` | Print text |
| `uname -a` | Show OS info |
| `whoami` | Print current user |
| `color <name>` | Change text color |
| `uptime` | Show system uptime |
| `reboot` | Reboot |
| `halt` | Halt the CPU |

---

## Build locally

```bash
# macOS
brew install i686-elf-gcc nasm qemu

# Build and run
make run-serial    # headless (terminal output)
make run           # QEMU window
```

See [BUILDING.md](BUILDING.md) for Linux/Windows instructions.

---

## Project structure

```
myos/
├── .devcontainer/          # GitHub Codespaces config
│   ├── devcontainer.json
│   ├── install-tools.sh    # Installs cross-compiler + QEMU
│   └── run.sh              # Auto-builds and launches on terminal open
├── boot/
│   ├── boot.asm            # Multiboot2 entry point, stack setup
│   ├── gdt_asm.asm         # GDT flush stub
│   └── isr_asm.asm         # ISR/IRQ assembly stubs (256 handlers)
├── kernel/
│   ├── kernel.c            # kernel_main() — boots everything
│   ├── gdt.c               # Global Descriptor Table
│   ├── idt.c               # Interrupt Descriptor Table + PIC remap
│   ├── isr.c               # Exception handler + IRQ dispatch
│   └── shell.c             # Interactive Unix-flavored shell
├── drivers/
│   ├── vga.c               # VGA text mode (80x25, 16 colors)
│   ├── keyboard.c          # PS/2 keyboard + serial input
│   └── serial.c            # COM1 serial port (for QEMU nographic)
├── include/                # Header files
├── iso/boot/grub/          # GRUB config for bootable ISO
├── linker.ld               # Places kernel at 1MB
└── Makefile
```

---

## Roadmap

- [x] Bootloader (GRUB/Multiboot2)
- [x] VGA text mode driver
- [x] GDT (memory segments)
- [x] IDT + interrupt handlers
- [x] PS/2 keyboard driver
- [x] Serial port driver (QEMU compatible)
- [x] Interactive shell with Unix commands
- [ ] Physical memory manager
- [ ] Paging / virtual memory
- [ ] Heap allocator (kmalloc/kfree)
- [ ] Filesystem (FAT32)
- [ ] `ls`, `cat`, `mkdir`, `pwd`, `touch`
- [ ] Multiple processes / scheduler

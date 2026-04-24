# Building MyOS

## Prerequisites

You need a **cross-compiler** — a version of GCC that targets bare metal x86,
not your host OS. This is mandatory; using your system GCC will produce binaries
that won't work.

---

## 1. Install the cross-compiler

### macOS (Homebrew)
```bash
brew install i686-elf-gcc nasm qemu xorriso
```

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 xorriso grub-pc-bin

# The cross-compiler isn't in apt — build it from source:
# (This takes ~10 minutes)
bash <(curl -s https://raw.githubusercontent.com/lordmilko/i686-elf-tools/master/i686-elf-tools-linux.sh)
```

### Windows
Use WSL2 (Ubuntu) and follow the Linux instructions above.

---

## 2. Build the kernel

```bash
make
```

This produces `myos.bin` — your kernel binary.

---

## 3. Run in QEMU (easiest)

```bash
make run
```

You should see the MyOS boot screen in a QEMU window!

---

## 4. Build a bootable ISO (optional)

```bash
make iso      # produces myos.iso
make run-iso  # boots the ISO in QEMU
```

You can also write `myos.iso` to a USB drive with `dd` and boot it on real hardware.

---

## Project structure

```
myos/
├── boot/
│   └── boot.asm        # Assembly entry point (Multiboot2 header + stack setup)
├── kernel/
│   └── kernel.c        # kernel_main() — the heart of the OS
├── drivers/
│   └── vga.c           # VGA text mode driver (80x25, 16 colors)
├── include/
│   └── vga.h           # VGA driver header
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg  # GRUB bootloader config
├── linker.ld           # Linker script (places kernel at 1MB)
├── Makefile
└── BUILDING.md         # This file
```

---

## What's next?

Once this boots, the next things to add are:

1. **GDT** — set up memory segments properly for 32-bit protected mode
2. **IDT + PIC** — handle CPU exceptions and hardware interrupts
3. **Keyboard driver** — read PS/2 keyboard via IRQ1
4. **PMM** — physical memory manager (track which pages are free)
5. **Paging** — virtual memory, process isolation
6. **Heap** — `kmalloc` / `kfree`
7. **Shell** — accept typed commands, execute them

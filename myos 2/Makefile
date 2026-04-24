# ============================================================
#  MyOS Makefile
#  Targets: all, iso, run, clean
# ============================================================

# --- Toolchain ---
# You need a cross-compiler: i686-elf-gcc and i686-elf-ld
# See BUILDING.md for installation instructions.
CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-ld
GRUB    = grub-mkrescue

# --- Flags ---
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

# --- Sources & Objects ---
C_SRCS   = kernel/kernel.c kernel/gdt.c kernel/idt.c kernel/isr.c \
           kernel/shell.c drivers/vga.c drivers/keyboard.c drivers/serial.c

ASM_SRCS = boot/boot.asm boot/gdt_asm.asm boot/isr_asm.asm

C_OBJS  = $(C_SRCS:.c=.o)
ASM_OBJS = $(ASM_SRCS:.asm=.o)
ALL_OBJS = $(ASM_OBJS) $(C_OBJS)

# --- Targets ---

.PHONY: all iso run clean

all: myos.bin

# Link everything into a kernel binary
myos.bin: $(ALL_OBJS) linker.ld
	$(CC) -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $(ALL_OBJS) -lgcc

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .asm files
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Build a bootable ISO (requires grub-mkrescue + xorriso)
iso: myos.bin
	cp myos.bin iso/boot/myos.bin
	$(GRUB) -o myos.iso iso
	@echo "ISO built: myos.iso"

# Run in QEMU (requires qemu-system-i386)
run: myos.bin
	qemu-system-i386 -kernel myos.bin

# For Codespaces / headless environments — output goes to terminal
# Exit with:  Ctrl+A  then  X
run-serial: myos.bin
	qemu-system-i386 -kernel myos.bin \
	    -nographic \
	    -serial mon:stdio \
	    -append "console=ttyS0"

# Run the ISO instead
run-iso: iso
	qemu-system-i386 -cdrom myos.iso

clean:
	rm -f $(ALL_OBJS) myos.bin myos.iso iso/boot/myos.bin

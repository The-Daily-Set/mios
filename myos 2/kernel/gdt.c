#include "gdt.h"

// The GDT table — 3 entries: null, kernel code, kernel data
static gdt_entry_t gdt[3];
static gdt_ptr_t   gdt_ptr;

// Pack a GDT entry from its component parts
static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran) {
    gdt[i].base_low    = (base & 0xFFFF);
    gdt[i].base_mid    = (base >> 16) & 0xFF;
    gdt[i].base_high   = (base >> 24) & 0xFF;
    gdt[i].limit_low   = (limit & 0xFFFF);
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access      = access;
}

// gdt_flush is in gdt_asm.asm — loads the GDTR and reloads segment registers
extern void gdt_flush(uint32_t gdt_ptr_addr);

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    // Entry 0: Null descriptor (required by CPU spec)
    gdt_set_entry(0, 0, 0, 0, 0);

    // Entry 1: Kernel code segment
    //   base=0, limit=4GB, ring 0, executable, readable
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Entry 2: Kernel data segment
    //   base=0, limit=4GB, ring 0, writable
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush((uint32_t)&gdt_ptr);
}

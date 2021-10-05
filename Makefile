TUPLE         := riscv64-unknown-elf-
CC            := $(TUPLE)gcc
OBJCPY        := $(TUPLE)objcopy
STRIP         := $(TUPLE)strip
CFLAGS        := -Wall -Wextra -std=c99 -O2 -march=rv64ima -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -nostartfiles -fno-stack-check -fno-stack-protector
LDFLAGS       := -static
DEFINES       := -D MM_FU540_C000

GFILES        := 
KFILES        := 

# Global Library
GFILES        := $(GFILES) src/inc/gcc_supp.o
GFILES        := $(GFILES) src/inc/string.o

# Kernel
#  - Core (Entry/System Setup/Globals)
KFILES        := $(KFILES) src/kernel/kernel.o
KFILES        := $(KFILES) src/kernel/kstart_entry.o
KFILES        := $(KFILES) src/kernel/globals.o
KFILES        := $(KFILES) src/kernel/memalloc.o
KFILES        := $(KFILES) src/kernel/debug.o
#  - Drivers
KFILES        := $(KFILES) src/kernel/drivers/uart.o
#  - Interrupt Handler
KFILES        := $(KFILES) src/kernel/interrupts/context_switch.o
KFILES        := $(KFILES) src/kernel/interrupts/context_switch_asm.o

.PHONY: all rebuild clean emu emu-debug debug

all: prog-emu.elf   prog-emu.elf.strip   prog-emu.elf.bin   prog-emu.elf.hex   prog-emu.elf.strip.bin   prog-emu.elf.strip.hex
#    prog-metal.elf prog-metal.elf.strip prog-metal.elf.bin prog-metal.elf.hex prog-metal.elf.strip.bin prog-metal.elf.strip.hex

rebuild: clean
	$(MAKE) all

clean:
	rm -f *.elf *.strip *.bin *.hex $(GFILES) $(KFILES)

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

prog-metal.elf: $(GFILES) $(KFILES)
	$(CC) $(CFLAGS) $(GFILES) $(KFILES) -T ./bare_metal.ld $(LDFLAGS) -o $@

prog-emu.elf: $(GFILES) $(KFILES)
	$(CC) $(CFLAGS) $(GFILES) $(KFILES) -T ./emulation.ld $(LDFLAGS) -o $@

prog-%.elf.strip: prog-%.elf
	$(STRIP) -s -x -R .comment -R .text.startup -R .riscv.attributes $^ -o $@

%.elf.bin: %.elf
	$(OBJCPY) -O binary $^ $@

%.elf.hex: %.elf
	$(OBJCPY) -O ihex $^ $@

%.strip.bin: %.strip
	$(OBJCPY) -O binary $^ $@

%.strip.hex: %.strip
	$(OBJCPY) -O ihex $^ $@

emu:
	qemu-system-riscv64 -bios ./prog-emu.elf.strip -M sifive_u -serial stdio -display none

emu-debug:
	qemu-system-riscv64 -bios ./prog-emu.elf.strip -M sifive_u -serial stdio -display none -gdb tcp::1234 -S

debug:
	$(TUPLE)gdb -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg general" -ex "break *0x80000000"

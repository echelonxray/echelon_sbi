TUPLE         := riscv64-unknown-elf-
CC            := $(TUPLE)gcc
OBJCPY        := $(TUPLE)objcopy
STRIP         := $(TUPLE)strip
CFLAGS        := -Wall -Wextra -std=c99 -O2 -march=rv64ima -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -nostartfiles -fno-stack-check -fno-stack-protector
LDFLAGS       := -static

GFILES        := 
KFILES        := 

# Global Library
GFILES        := $(GFILES) src/inc/string.o src/inc/gcc_supp.o

# Kernel
#  - Core (Entry/System Setup/Globals)
KFILES        := $(KFILES) src/kernel/kernel.o
KFILES        := $(KFILES) src/kernel/kstart_entry.o
KFILES        := $(KFILES) src/kernel/globals.o
KFILES        := $(KFILES) src/kernel/memalloc.o
KFILES        := $(KFILES) src/kernel/debug.o
#  - Drivers
KFILES        := $(KFILES) src/kernel/drivers/uart.o
#  - Idle Loop
KFILES        := $(KFILES) src/kernel/idle/idle_loop.o
#  - Interrupt Handler
KFILES        := $(KFILES) src/kernel/interrupts/interrupt.o
KFILES        := $(KFILES) src/kernel/interrupts/interrupt_entry.o
KFILES        := $(KFILES) src/kernel/interrupts/context_switch_asm.o

.PHONY: all rebuild clean emu emu-debug debug

all: prog-emu.elf   prog-emu.elf.strip   prog-emu.elf.bin   prog-emu.elf.hex   prog-emu.elf.strip.bin   prog-emu.elf.strip.hex
#    prog-metal.elf prog-metal.elf.strip prog-metal.elf.bin prog-metal.elf.hex prog-metal.elf.strip.bin prog-metal.elf.strip.hex

rebuild: clean
	$(MAKE) all

clean:
	rm -f *.elf *.strip *.bin *.hex $(GFILES) $(KFILES)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $^ -c -o $@

prog-metal.elf: $(GFILES) $(KFILES)
	$(CC) $(CFLAGS) $(GFILES) $(KFILES) -T ./bare_metal.ld $(LDFLAGS) -o $@

prog-emu.elf: $(GFILES) $(KFILES) #$(UFILES)
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

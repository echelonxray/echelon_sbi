CC      := riscv32-unknown-elf-gcc
OBJCPY  := riscv32-unknown-elf-objcopy
STRIP   := riscv32-unknown-elf-strip
CFLAGS  := -Wall -Wextra -std=c99 -O2 -march=rv32imac -mabi=ilp32 -ffreestanding -nostdlib -nostartfiles -Wno-unused-parameter
LDFLAGS := -T ./linker_script.ld
GFILES  := 
KFILES  := 
UFILES  := 

# Global Library
GFILES  := $(GFILES) ./src/inc/string.o

# Kernel
#  - Core (Entry/System Setup/Globals)
KFILES  := $(KFILES) ./src/kernel/base.o
KFILES  := $(KFILES) ./src/kernel/entry.o
KFILES  := $(KFILES) ./src/kernel/globals.o
KFILES  := $(KFILES) ./src/kernel/memalloc.o
KFILES  := $(KFILES) ./src/kernel/debug.o
#  - Drivers
KFILES  := $(KFILES) ./src/kernel/drivers/uart.o
#  - Idle Loop
KFILES  := $(KFILES) ./src/kernel/idle/idle_loop.o
#  - Interrupt Handler
KFILES  := $(KFILES) ./src/kernel/interrupts/base.o
KFILES  := $(KFILES) ./src/kernel/interrupts/entry.o
KFILES  := $(KFILES) ./src/kernel/interrupts/context_switch_asm.o
KFILES  := $(KFILES) ./src/kernel/interrupts/context_switch.o

# Programs
#  - Init
UFILES  := $(UFILES) ./src/progs/init/init.o

.PHONY: all rebuild clean

all: prog.elf prog.bin prog.hex prog-strip.elf prog-strip.bin prog-strip.hex

clean:
	rm -f *.elf *.bin *.hex $(GFILES) $(KFILES) $(UFILES)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $^ -c -o $@

prog.elf: $(GFILES) $(KFILES) $(UFILES)
	$(CC) -static $(CFLAGS) $^ $(LDFLAGS) -o $@

prog-strip.elf: prog.elf
	$(STRIP) -s -x -R .comment -R .text.startup -R .riscv.attributes $^ -o $@

%.bin: %.elf
	$(OBJCPY) -O binary $^ $@

%.hex: %.elf
	$(OBJCPY) -O ihex $^ $@

TUPLE         := riscv32-unknown-elf
CC            := $(TUPLE)-gcc
OBJCPY        := $(TUPLE)-objcopy
STRIP         := $(TUPLE)-strip
CFLAGS        := -Wall -Wextra -std=c99 -O2 -march=rv32imac -mabi=ilp32 -ffreestanding -nostdlib -nostartfiles -Wno-unused-parameter -fno-stack-check -fno-stack-protector
LDFLAGS       := -static
FS_MOUNT_PATH := $(PWD)/tmp_mnt_pt

GFILES        := 
KFILES        := 
UFILES        := 

# Global Library
GFILES        := $(GFILES) src/inc/string.o

# Kernel
#  - Core (Entry/System Setup/Globals)
KFILES        := $(KFILES) src/kernel/base.o
KFILES        := $(KFILES) src/kernel/entry.o
KFILES        := $(KFILES) src/kernel/globals.o
KFILES        := $(KFILES) src/kernel/memalloc.o
KFILES        := $(KFILES) src/kernel/debug.o
#  - Drivers
KFILES        := $(KFILES) src/kernel/drivers/uart.o
#  - Idle Loop
KFILES        := $(KFILES) src/kernel/idle/idle_loop.o
#  - Interrupt Handler
KFILES        := $(KFILES) src/kernel/interrupts/base.o
KFILES        := $(KFILES) src/kernel/interrupts/entry.o
KFILES        := $(KFILES) src/kernel/interrupts/context_switch_asm.o
KFILES        := $(KFILES) src/kernel/interrupts/context_switch.o

# Programs
#  - Init
#UFILES        := $(UFILES) ./src/progs/init/init.o

export TUPLE
export CC
export OBJCPY
export STRIP
export CFLAGS
export LDFLAGS
export FS_MOUNT_PATH
export GFILES

.PHONY: all rebuild clean userspace emu emu-debug debug

all: prog-metal.elf prog-metal.elf.strip prog-metal.elf.bin prog-metal.elf.hex prog-metal.elf.strip.bin prog-metal.elf.strip.hex \
     prog-emu.elf   prog-emu.elf.strip   prog-emu.elf.bin   prog-emu.elf.hex   prog-emu.elf.strip.bin   prog-emu.elf.strip.hex

rebuild: clean
	$(MAKE) all

clean:
	rm -f *.elf *.strip *.bin *.hex $(GFILES) $(KFILES) #$(UFILES)
	$(MAKE) -C ./src/progs clean
	rm -rf $(FS_MOUNT_PATH)/../userspace.cpio $(FS_MOUNT_PATH) userspace.cpio.elf

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $^ -c -o $@

prog-metal.elf: $(GFILES) $(KFILES) userspace #$(UFILES)
	$(CC) $(CFLAGS) $(GFILES) $(KFILES) userspace.cpio.elf -T ./bare_metal.ld $(LDFLAGS) -o $@

prog-emu.elf: $(GFILES) $(KFILES) userspace #$(UFILES)
	$(CC) $(CFLAGS) $(GFILES) $(KFILES) userspace.cpio.elf -T ./emulation.ld $(LDFLAGS) -o $@

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

userspace:
	mkdir -p $(FS_MOUNT_PATH)
	$(MAKE) -C ./src/progs all
	$(MAKE) -C $(FS_MOUNT_PATH) -f $(PWD)/Makefile userspace.cpio
	$(OBJCPY) -I binary -B riscv -O elf32-littleriscv \
                  --rename-section .data=section_CPIO_ARCHIVE,alloc,load,readonly,data,contents \
                  $(FS_MOUNT_PATH)/../userspace.cpio userspace.cpio.elf

userspace.cpio:
	find . | cpio -o -H bin > $(FS_MOUNT_PATH)/../$@

emu:
	qemu-system-riscv32 -kernel ./prog-emu.elf.strip -M sifive_e -serial stdio -display none

emu-debug:
	qemu-system-riscv32 -kernel ./prog-emu.elf.strip -M sifive_e -serial stdio -display none -gdb tcp::1234 -S

debug:
	$(TUPLE)-gdb -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg general" -ex "break *0x20400000" -ex "continue"

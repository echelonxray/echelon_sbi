TUPLE         := riscv64-unknown-elf-
CC            := $(TUPLE)gcc
OBJCPY        := $(TUPLE)objcopy
STRIP         := $(TUPLE)strip
LDFLAGS       := -e my_entry_pt -Wl,-gc-sections -static
DEFINES       := -D MM_FU540_C000
CFLAGS        := 
CFLAGS        := $(CFLAGS) -Wall -Wextra # Turn on all buuld warnings.
CFLAGS        := $(CFLAGS) -std=c99 # The standards to build to.
CFLAGS        := $(CFLAGS) -march=rv64ia -mabi=lp64 # The build target architectural information.
CFLAGS        := $(CFLAGS) -mcmodel=medany # The symbol relocation scheme.
CFLAGS        := $(CFLAGS) -O2 -mrelax -fno-stack-check -fno-stack-protector -fomit-frame-pointer # Optimizations to make and unused features/cruft.
CFLAGS        := $(CFLAGS) -ftls-model=local-exec # Thread Local Store (TLS) scheme: Final TLS offsets are known at linktime. (local-exec)
CFLAGS        := $(CFLAGS) -fno-pic # Do not build position independent code.  Older versions of GCC did not default to this.
CFLAGS        := $(CFLAGS) -ffreestanding -nostdlib -nostartfiles # Build a freestanding program.  Do not automatically include any other libraries or object files.
CFLAGS        := $(CFLAGS) -fno-zero-initialized-in-bss # Because this will run on the bare metal, there is nothing to zero the memory.  Do not assume that fresh memory is zeroed.

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
KFILES        := $(KFILES) src/kernel/thread_locking.o
KFILES        := $(KFILES) src/kernel/debug.o
#  - Drivers
KFILES        := $(KFILES) src/kernel/drivers/uart.o
#  - Interrupt Handler
KFILES        := $(KFILES) src/kernel/interrupts/context_switch.o
KFILES        := $(KFILES) src/kernel/interrupts/context_switch_asm.o

.PHONY: all rebuild clean supervisorspace emu emu-debug debug

all: prog-emu.elf   prog-emu.elf.strip   prog-emu.elf.bin   prog-emu.elf.hex   prog-emu.elf.strip.bin   prog-emu.elf.strip.hex
#    prog-metal.elf prog-metal.elf.strip prog-metal.elf.bin prog-metal.elf.hex prog-metal.elf.strip.bin prog-metal.elf.strip.hex

rebuild: clean
	$(MAKE) all

clean:
	rm -f *.elf *.strip *.bin *.hex prog-partial.o prog-prerelax.o $(GFILES) $(KFILES)
	$(MAKE) -C ./test clean

supervisorspace:
	$(MAKE) -C ./test all

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

prog-partial.o: $(GFILES) $(KFILES) supervisorspace
	$(CC) $(CFLAGS) $(GFILES) $(KFILES) -r $(LDFLAGS) -o $@

prog-prerelax.o: prog-partial.o
	$(OBJCPY) --set-section-flags .rodata.str1.8=alloc $^ $@

prog-metal.elf: prog-prerelax.o
	$(CC) $(CFLAGS) $^ -T ./bare_metal.ld $(LDFLAGS) -o $@

prog-emu.elf: prog-prerelax.o
	$(CC) $(CFLAGS) $^ -T ./emulation.ld $(LDFLAGS) -o $@

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
	qemu-system-riscv64 -cpu sifive-u54 -smp 5 -bios ./prog-emu.elf.strip.bin -M sifive_u -serial stdio -display none -device loader,file=./test/test.out.strip.bin,addr=0x20000000

emu-debug:
	qemu-system-riscv64 -cpu sifive-u54 -smp 5 -bios ./prog-emu.elf.strip.bin -M sifive_u -serial stdio -display none -device loader,file=./test/test.out.strip.bin,addr=0x20000000 -gdb tcp::1234 -S

emu-linux:
	qemu-system-riscv64 -cpu sifive-u54 -smp 5 -kernel ./kernel.img -M sifive_u -serial stdio -display none

debug:
	$(TUPLE)gdb -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg general" -ex "break *0x80000000"

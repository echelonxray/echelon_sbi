TUPLE         := riscv64-unknown-elf-
#TUPLE         := riscv32-unknown-linux-gnu-
CC            := $(TUPLE)gcc
OBJCPY        := $(TUPLE)objcopy
STRIP         := $(TUPLE)strip
LDFLAGS       := -e my_entry_pt -Wl,-gc-sections -static
#DEFINES       := -D MM_QEMU_VIRT
#DEFINES       := -D MM_CUSTOM_EMU
DEFINES       :=
CFLAGS        :=
CFLAGS        := $(CFLAGS) -Wall -Wextra # Set build warnings
CFLAGS        := $(CFLAGS) -std=c99 # The standards to build to.
CFLAGS        := $(CFLAGS) -march=rv32ia -mabi=ilp32 -mlittle-endian -mstrict-align # The build target architectural information.
CFLAGS        := $(CFLAGS) -mcmodel=medany # The symbol relocation scheme.
CFLAGS        := $(CFLAGS) -O3 -mrelax -fno-stack-check -fno-stack-protector -fomit-frame-pointer # Optimizations to make and unused features/cruft.
CFLAGS        := $(CFLAGS) -ftls-model=local-exec # Thread Local Store (TLS) scheme: Final TLS offsets are known at linktime. (local-exec)
CFLAGS        := $(CFLAGS) -fno-pic # Do not build position independent code.  Older versions of GCC did not default to this.
CFLAGS        := $(CFLAGS) -ffreestanding -nostdlib -nostartfiles # Build a freestanding program.  Do not automatically include any other libraries or object files.
CFLAGS        := $(CFLAGS) -fno-zero-initialized-in-bss # Because this will run on the bare metal, there is nothing to zero the memory.  Do not assume that fresh memory is zeroed.
CFLAGS        := $(CFLAGS) -MD # Generate header dependency tracking information

GFILES        :=
KFILES        :=

# Global Library
GFILES        := $(GFILES) src/inc/gcc_supp.o
GFILES        := $(GFILES) src/inc/string.o

# Kernel
#  - Core (Entry/System Setup/Globals)
KFILES        := $(KFILES) src/kernel/kernel.o
KFILES        := $(KFILES) src/kernel/cpio_parse.o
KFILES        := $(KFILES) src/kernel/dtb_parse.o
KFILES        := $(KFILES) src/kernel/kstart_entry.o
KFILES        := $(KFILES) src/kernel/globals.o
KFILES        := $(KFILES) src/kernel/memalloc.o
KFILES        := $(KFILES) src/kernel/thread_locking.o
KFILES        := $(KFILES) src/kernel/sbi_commands.o
KFILES        := $(KFILES) src/kernel/debug.o
#  - Drivers
KFILES        := $(KFILES) src/kernel/drivers/uart.o
#  - Interrupt Handler
KFILES        := $(KFILES) src/kernel/interrupts/context_switch.o
KFILES        := $(KFILES) src/kernel/interrupts/context_switch_asm.o
#  - SBI Commands
KFILES        := $(KFILES) src/kernel/sbi_commands/base.o
KFILES        := $(KFILES) src/kernel/sbi_commands/time.o
KFILES        := $(KFILES) src/kernel/sbi_commands/ipi.o
KFILES        := $(KFILES) src/kernel/sbi_commands/rfnc.o
KFILES        := $(KFILES) src/kernel/sbi_commands/hsm.o

FILES_O       := $(GFILES) $(KFILES)
FILES_D       := $(addsuffix .d,$(basename $(FILES_O)))

.PHONY: all cust virt files clean emu emu-debug emu-linux emu-linux-debug emu-opensbi-linux emu-opensbi-linux-debug debug

all: cust

cust: clean
	$(MAKE) files DEFINES="-D MM_CUSTOM_EMU"

virt: clean
	$(MAKE) files DEFINES="-D MM_QEMU_VIRT"

files: prog-emu.elf   prog-emu.elf.strip   prog-emu.elf.bin   prog-emu.elf.hex   prog-emu.elf.strip.bin   prog-emu.elf.strip.hex
#    prog-metal.elf prog-metal.elf.strip prog-metal.elf.bin prog-metal.elf.hex prog-metal.elf.strip.bin prog-metal.elf.strip.hex

clean:
	rm -f *.elf *.strip *.bin *.hex prog-partial.o prog-prerelax.o $(FILES_O) $(FILES_D)

%.o: %.c $(wildcard %.d)
	$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

%.o: %.S $(wildcard %.d)
	$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

prog-partial.o: $(FILES_O)
	$(CC) $(CFLAGS) $^ -r $(LDFLAGS) -o $@

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
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -bios ./prog-emu.elf.strip.bin

emu-debug:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -bios ./prog-emu.elf.strip.bin -gdb tcp::1234 -S

emu-linux:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb -bios ./prog-emu.elf.strip.bin

emu-linux-debug:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb -bios ./prog-emu.elf.strip.bin -gdb tcp::1234 -S

emu-opensbi-linux:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb

emu-opensbi-linux-debug:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb -gdb tcp::1234 -S

debug:
	$(TUPLE)gdb -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg general" -ex "break *0x80000000"

-include $(wildcard $(FILES_D))

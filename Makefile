TRIPLET       := riscv64-ex3-linux-musl-
#TRIPLET      := riscv32-unknown-linux-gnu-
CC            := $(TRIPLET)gcc
#CC            := clang -target riscv64
OBJCPY        := $(TRIPLET)objcopy
#OBJCPY        := llvm-objcopy
STRIP         := $(TRIPLET)strip
#STRIP         := llvm-strip
LDFLAGS       := -e my_entry_pt -Wl,-gc-sections -static
CFLAGS        :=
CFLAGS        := $(CFLAGS) -Wall -Wextra -g # Set build warnings and debugging
CFLAGS        := $(CFLAGS) -std=c99 # The standards to build to.
CFLAGS        := $(CFLAGS) -march=rv32ia_zicsr_zifencei -mabi=ilp32 -mlittle-endian -mstrict-align # The build target architectural information.
#CFLAGS        := $(CFLAGS) -march=rv32ia -mabi=ilp32 -mlittle-endian # The build target architectural information.
CFLAGS        := $(CFLAGS) -mcmodel=medany # The symbol relocation scheme.
CFLAGS        := $(CFLAGS) -O3 -mrelax -fno-stack-check -fno-stack-protector -fomit-frame-pointer -ffunction-sections # Optimizations to make and unused features/cruft.
CFLAGS        := $(CFLAGS) -ftls-model=local-exec # Thread Local Store (TLS) scheme: Final TLS offsets are known at linktime. (local-exec)
CFLAGS        := $(CFLAGS) -fno-pic -fno-pie # Do not build position independent code.  Older versions of GCC did not default to this.
CFLAGS        := $(CFLAGS) -ffreestanding -nostdlib # Build a freestanding program.  Do not automatically include any other libraries or object files.
CFLAGS        := $(CFLAGS) -fno-zero-initialized-in-bss # Because this will run on the bare metal, there is nothing to zero the memory.  Do not assume that fresh memory is zeroed.
CFLAGS        := $(CFLAGS) -MD # Generate header dependency tracking information

TGT_SFXs      := .elf .elf.bin .elf.hex .elf.strip .elf.strip.bin .elf.strip.hex

DEFINES       :=
TAG           :=

GFILES        :=
KFILES        :=

# Global Library
GFILES        := $(GFILES) src/inc/gcc_supp.?
GFILES        := $(GFILES) src/inc/string.?

# Kernel
#  - Core (Entry/System Setup/Globals)
KFILES        := $(KFILES) src/kernel/kernel.?
KFILES        := $(KFILES) src/kernel/cpio_parse.?
KFILES        := $(KFILES) src/kernel/dtb_parse.?
KFILES        := $(KFILES) src/kernel/kstart_entry.?
KFILES        := $(KFILES) src/kernel/globals.?
KFILES        := $(KFILES) src/kernel/memalloc.?
KFILES        := $(KFILES) src/kernel/thread_locking.?
KFILES        := $(KFILES) src/kernel/sbi_commands.?
KFILES        := $(KFILES) src/kernel/debug.?
#  - Drivers
KFILES        := $(KFILES) src/kernel/drivers/uart.?
#  - Interrupt Handler
KFILES        := $(KFILES) src/kernel/interrupts/context_switch.?
KFILES        := $(KFILES) src/kernel/interrupts/context_switch_asm.?
#  - SBI Commands
KFILES        := $(KFILES) src/kernel/sbi_commands/base.?
KFILES        := $(KFILES) src/kernel/sbi_commands/time.?
KFILES        := $(KFILES) src/kernel/sbi_commands/ipi.?
KFILES        := $(KFILES) src/kernel/sbi_commands/rfnc.?
KFILES        := $(KFILES) src/kernel/sbi_commands/hsm.?

# What list of base filenames are we to build?
FILES_BASE    := $(basename $(GFILES) $(KFILES))

.PHONY: all clean echelon_emu qemu_virt emu emu-debug emu-linux emu-linux-debug emu-opensbi-linux emu-opensbi-linux-debug debug

.PRECIOUS: %.o %.$(TAG).o

# Primary make targets.  These are the targets that can be called directly.

all: echelon_emu qemu_virt

rebuild: clean
	$(MAKE) all

clean:
	rm -f esbi-*
	rm -f $(addsuffix .o,$(FILES_BASE))
	rm -f $(addsuffix .d,$(FILES_BASE))
	rm -f $(addsuffix .echelon_emu.o,$(FILES_BASE))
	rm -f $(addsuffix .echelon_emu.d,$(FILES_BASE))
	rm -f $(addsuffix .qemu_virt.o,$(FILES_BASE))
	rm -f $(addsuffix .qemu_virt.d,$(FILES_BASE))

echelon_emu:
	$(MAKE) $(addprefix esbi-$@,$(TGT_SFXs)) DEFINES="-D MM_CUSTOM_EMU" TAG="$@"

qemu_virt:
	$(MAKE) $(addprefix esbi-$@,$(TGT_SFXs)) DEFINES="-D MM_QEMU_VIRT" TAG="$@"

# Targets below this line should not be called directly.

# Root make targets

esbi-echelon_emu.elf: $(addsuffix .$(TAG).o,$(FILES_BASE))
	$(CC) $(CFLAGS) $^ -T ./emulation.ld $(LDFLAGS) -o $@

esbi-qemu_virt.elf: $(addsuffix .$(TAG).o,$(FILES_BASE))
	$(CC) $(CFLAGS) $^ -T ./emulation.ld $(LDFLAGS) -o $@

# File building

%.$(TAG).o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

%.$(TAG).o: %.S
	$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

# Build output variants

%.strip: %
	$(STRIP) -s -x -R .comment -R .text.startup -R .riscv.attributes $^ -o $@

%.bin: %
	$(OBJCPY) -O binary $^ $@

%.hex: %
	$(OBJCPY) -O ihex $^ $@

# Header dependency tracking

-include $(addsuffix .$(TAG).d,$(FILES_BASE))

# Testing and debugging (These can be called directly)

emu:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -bios ./esbi-qemu_virt.elf.strip.bin

emu-debug:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -bios ./esbi-qemu_virt.elf.strip.bin -gdb tcp::1234 -S

emu-linux:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb -bios ./esbi-qemu_virt.elf.strip.bin

emu-linux-debug:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb -bios ./esbi-qemu_virt.elf.strip.bin -gdb tcp::1234 -S

emu-opensbi-linux:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb

emu-opensbi-linux-debug:
	qemu-system-riscv32 -M virt -cpu rv32 -smp 1 -m 128M -serial stdio -display none -kernel ./ignore/emudata/riscv32iam_linux_kernel.bin -initrd ./ignore/emudata/fs.cpio.gz -append "rdinit=/init.out loglevel=15" -dtb ./ignore/emudata/jsem.dtb -gdb tcp::1234 -S

debug:
	$(TRIPLET)gdb -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg general" -ex "break *0x80000000"

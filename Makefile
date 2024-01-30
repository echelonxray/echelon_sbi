ifeq ($(origin CC),default)
  undefine CC
endif
ifeq ($(origin LD),default)
  undefine LD
endif

LOCAL_CFLAGS  :=
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -Wall -Wextra -g # Set build warnings and debugging
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -std=c99 # The standards to build to.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -march=rv32ia_zicsr_zifencei -mabi=ilp32 -mlittle-endian -mstrict-align # The build target architectural information.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -mcmodel=medany # The symbol relocation scheme.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -O3 -mrelax -fno-stack-check -fno-stack-protector -fomit-frame-pointer -ffunction-sections # Optimizations to make and unused features/cruft.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -ftls-model=local-exec # Thread Local Store (TLS) scheme: Final TLS offsets are known at linktime. (local-exec)
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -fno-pic -fno-pie # Do not build position independent code.  Older versions of GCC did not default to this.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -nostdinc -I./src # Don't include system header files.  Use local headers.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -ffreestanding -nostdlib # Build a freestanding program.  Do not automatically include any other libraries or object files.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -fno-zero-initialized-in-bss # Because this will run on the bare metal, there is nothing to zero the memory.  Do not assume that fresh memory is zeroed.
LOCAL_CFLAGS  := $(LOCAL_CFLAGS) -MMD # Generate header dependency tracking information

LOCAL_LDFLAGS :=
LOCAL_LDFLAGS := $(LOCAL_LDFLAGS) -b elf32-littleriscv -e my_entry_pt --gc-sections -static

CROSS_COMPILE ?= riscv32-rv32ia-linux-musl-
CFLAGS        ?= $(LOCAL_CFLAGS)
LDFLAGS       ?= $(LOCAL_LDFLAGS)
CC            ?= gcc
LD            ?= ld
OBJCPY        ?= objcopy
STRIP         ?= strip

TGT_SFXs      := .elf .elf.bin .elf.hex .elf.strip .elf.strip.bin .elf.strip.hex

DEFINES       :=
TAG           :=

FILES         :=

# Core (Entry/System Setup/Globals)
FILES         := $(FILES) src/main.?
FILES         := $(FILES) src/entry.?
FILES         := $(FILES) src/globals.?
FILES         := $(FILES) src/memalloc.?
FILES         := $(FILES) src/thread_locking.?
FILES         := $(FILES) src/sbi_commands.?
FILES         := $(FILES) src/printm.?
# Data Parsing
FILES         := $(FILES) src/dataparse/cpio_parse.?
FILES         := $(FILES) src/dataparse/dtb_parse.?
# Drivers
FILES         := $(FILES) src/drivers/uart.?
# Interrupt Handler
FILES         := $(FILES) src/interrupts/context_switch.?
FILES         := $(FILES) src/interrupts/context_switch_asm.?
# SBI Commands
FILES         := $(FILES) src/sbi_commands/base.?
FILES         := $(FILES) src/sbi_commands/time.?
FILES         := $(FILES) src/sbi_commands/ipi.?
FILES         := $(FILES) src/sbi_commands/rfnc.?
FILES         := $(FILES) src/sbi_commands/hsm.?
FILES         := $(FILES) src/sbi_commands/srst.?
# Misc
FILES         := $(FILES) src/gcc_supp.?
FILES         := $(FILES) src/string.?

# What list of base filenames are we to build?
FILES_BASE    := $(basename $(FILES))

.PHONY: all rebuild clean echelon_emu qemu_virt emu emu-debug emu-linux emu-linux-debug emu-opensbi-linux emu-opensbi-linux-debug debug

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
	$(CROSS_COMPILE)$(LD) $^ -T ./linking.ld $(LDFLAGS) -o $@

esbi-qemu_virt.elf: $(addsuffix .$(TAG).o,$(FILES_BASE))
	$(CROSS_COMPILE)$(LD) $^ -T ./linking.ld $(LDFLAGS) -o $@

# File compiling

%.$(TAG).o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

%.$(TAG).o: %.S
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $< -c -o $@

# Build output variants

%.strip: %
	$(CROSS_COMPILE)$(STRIP) -s -x -R .comment -R .text.startup -R .riscv.attributes $^ -o $@

%.bin: %
	$(CROSS_COMPILE)$(OBJCPY) -O binary $^ $@

%.hex: %
	$(CROSS_COMPILE)$(OBJCPY) -O ihex $^ $@

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
	$(CROSS_COMPILE)gdb -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg general" -ex "break *0x80000000"

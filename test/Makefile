TUPLE         := riscv64-unknown-elf-
CC            := $(TUPLE)gcc
OBJCPY        := $(TUPLE)objcopy
STRIP         := $(TUPLE)strip
LDFLAGS       := -e my_entry_pt -Wl,-gc-sections -static
DEFINES       := 
CFLAGS        := 
CFLAGS        := $(CFLAGS) -Wall -Wextra # Turn on all buuld warnings.
CFLAGS        := $(CFLAGS) -std=c99 # The standards to build to.
CFLAGS        := $(CFLAGS) -march=rv64ia -mabi=lp64 # The build target architectural information.
CFLAGS        := $(CFLAGS) -mcmodel=medany # The symbol relocation scheme.
CFLAGS        := $(CFLAGS) -O2 -mno-relax -fno-stack-check -fno-stack-protector -fomit-frame-pointer # Optimizations to make and unused features/cruft.
CFLAGS        := $(CFLAGS) -ftls-model=local-exec # Thread Local Store (TLS) scheme: Final TLS offsets are known at linktime. (local-exec)
CFLAGS        := $(CFLAGS) -fno-pic # Do not build position independent code.  Older versions of GCC did not default to this.
CFLAGS        := $(CFLAGS) -ffreestanding -nostdlib -nostartfiles # Build a freestanding program.  Do not automatically include any other libraries or object files.
CFLAGS        := $(CFLAGS) -fno-zero-initialized-in-bss # Because this will run on the bare metal, there is nothing to zero the memory.  Do not assume that fresh memory is zeroed.

BFILES        := 

# Files to build
BFILES        := $(BFILES) test.o

.PHONY: all rebuild clean

all: test.out   test.out.strip   test.out.bin   test.out.strip.bin

rebuild: clean
	$(MAKE) all

clean:
	rm -f *.out *.strip *.bin $(BFILES)

%.o: %.s
	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

#%.o: %.S
#	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

#%.o: %.c
#	$(CC) $(CFLAGS) $(DEFINES) $^ -c -o $@

test.out: $(BFILES)
	$(CC) $(CFLAGS) $^ -T ./test.ld $(LDFLAGS) -o $@

%.out.strip: %.out
	$(STRIP) -s -x -R .comment -R .text.startup -R .riscv.attributes $^ -o $@

%.out.bin: %.out
	$(OBJCPY) -O binary $^ $@

%.strip.bin: %.strip
	$(OBJCPY) -O binary $^ $@

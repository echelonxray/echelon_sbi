CC      := riscv32-unknown-linux-elf-gcc
CFLAGS  := -Wall -Wextra -std=c99 -O2 -march=rv32ima -mabi=ilp32 -ffreestanding -nostdlib -nostartfiles -Wno-unused-parameter
LDFLAGS := -T ./hello_world.ld
FILES   := hello_world.o hello_world_entry.o

.PHONY: all rebuild clean ihex

all: prog ihex

rebuild: clean all

clean:
	rm -f prog prog.hex *.o

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $^ -c -o $@

prog: $(FILES)
	$(CC) -static $(CFLAGS) $^ $(LDFLAGS) -o $@

ihex:
	riscv32-unknown-linux-elf-objcopy -O ihex ./prog ./prog.hex

CC      := riscv32-unknown-elf-gcc
OBJCPY  := riscv32-unknown-elf-objcopy
CFLAGS  := -Wall -Wextra -std=c99 -O2 -march=rv32ima -mabi=ilp32 -ffreestanding -nostdlib -nostartfiles -Wno-unused-parameter
LDFLAGS := -T ./hello_world.ld
FILES   := ./src/hello_world.o ./src/hello_world_entry.o

.PHONY: all rebuild clean ihex

all: prog ihex

rebuild: clean all

clean:
	rm -f ./prog ./prog.hex ./*.o ./src/*.o $(FILES)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $^ -c -o $@

prog: $(FILES)
	$(CC) -static $(CFLAGS) $^ $(LDFLAGS) -o $@

ihex:
	$(OBJCPY) -O ihex ./prog ./prog.hex

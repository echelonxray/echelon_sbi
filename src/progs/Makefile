UPROGS        := init

CFLAGS        := -Wall -Wextra -std=c99 -O2 -march=rv32imac -mabi=ilp32 -ffreestanding -nostdlib -nostartfiles -fno-stack-check -fno-stack-protector -fpic
LDFLAGS       := -static

.PHONY: all clean build-all-progs clean-all-progs

all: build-all-progs

clean: clean-all-progs

build-all-progs: $(UPROGS)
	$(MAKE) -C $^ all
	$(MAKE) -C $^ cp_to_fs_mnt

clean-all-progs: $(UPROGS)
	$(MAKE) -C $^ clean
	$(MAKE) -C $^ del_from_fs_mnt

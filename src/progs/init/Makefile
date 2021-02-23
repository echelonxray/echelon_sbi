UFILES    := init.o
BUILDNAME := init.out

.PHONY: all clean cp_to_fs_mnt del_from_fs_mnt

all: $(BUILDNAME)

clean:
	rm -rf $(BUILDNAME) $(UFILES)

$(BUILDNAME): $(UFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	$(STRIP) -s -x -R .comment -R .text.startup -R .riscv.attributes $@

cp_to_fs_mnt: $(BUILDNAME)
	cp $(BUILDNAME) $(FS_MOUNT_PATH)/$(BUILDNAME)

del_from_fs_mnt:
	rm -rf $(FS_MOUNT_PATH)/$(BUILDNAME)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

%.o: %.s
	$(CC) $(CFLAGS) $^ -c -o $@

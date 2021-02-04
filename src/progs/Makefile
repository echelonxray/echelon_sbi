UPROGS := init

.PHONY: all clean build-all-progs clean-all-progs

all: build-all-progs

clean: clean-all-progs

build-all-progs: $(UPROGS)
	$(MAKE) -C $^ all
	$(MAKE) -C $^ cp_to_fs_mnt

clean-all-progs: $(UPROGS)
	$(MAKE) -C $^ clean
	$(MAKE) -C $^ del_from_fs_mnt

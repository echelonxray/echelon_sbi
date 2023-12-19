#ifndef _insertion_dataparse_cpio_parse_h
#define _insertion_dataparse_cpio_parse_h

#include <inc/types.h>

struct __attribute__((packed)) bits16 {
	uint16_t hi16;
	uint16_t lw16;
};
union hwords {
	struct bits16 hwds;
	uint32_t vl32;
};
struct __attribute__((packed)) header_pwb_cpio {
	uint16_t h_magic;
	uint16_t h_dev;
	uint16_t h_ino;
	uint16_t h_mode;
	uint16_t h_uid;
	uint16_t h_gid;
	uint16_t h_nlink;
	uint16_t h_majmin;
	union hwords h_mtime;
	uint16_t h_namesize;
	union hwords h_filesize;
};

void* get_cpio_entry_header(char* filename, void* cpio_archive, struct header_pwb_cpio* buffer);

#endif

#include "./cpio_parse.h"
#include "./../inc/types.h"
#include "./../inc/string.h"

uint16_t cpio_parse_swap_endianess_16(uint16_t val) {
	uint16_t retval;
	retval  = (val << 8) & 0xFF00;
	retval |= (val >> 8) & 0x00FF;
	return retval;
}

void read_entry(struct header_pwb_cpio* cpio_head) {
	if (cpio_head->h_magic != 0x71C7) {
		cpio_head->h_magic = cpio_parse_swap_endianess_16(cpio_head->h_magic);
		cpio_head->h_dev = cpio_parse_swap_endianess_16(cpio_head->h_dev);
		cpio_head->h_ino = cpio_parse_swap_endianess_16(cpio_head->h_ino);
		cpio_head->h_mode = cpio_parse_swap_endianess_16(cpio_head->h_mode);
		cpio_head->h_uid = cpio_parse_swap_endianess_16(cpio_head->h_uid);
		cpio_head->h_gid = cpio_parse_swap_endianess_16(cpio_head->h_gid);
		cpio_head->h_nlink = cpio_parse_swap_endianess_16(cpio_head->h_nlink);
		cpio_head->h_majmin = cpio_parse_swap_endianess_16(cpio_head->h_majmin);
		cpio_head->h_mtime.hwds.hi16 = cpio_parse_swap_endianess_16(cpio_head->h_mtime.hwds.hi16);
		cpio_head->h_mtime.hwds.lw16 = cpio_parse_swap_endianess_16(cpio_head->h_mtime.hwds.lw16);
		cpio_head->h_namesize = cpio_parse_swap_endianess_16(cpio_head->h_namesize);
		cpio_head->h_filesize.hwds.hi16 = cpio_parse_swap_endianess_16(cpio_head->h_filesize.hwds.hi16);
		cpio_head->h_filesize.hwds.lw16 = cpio_parse_swap_endianess_16(cpio_head->h_filesize.hwds.lw16);
	}
	
	uint16_t tmpval = 0x0123;
	uint8_t* ptrval = (void*)(&tmpval);
	if (*ptrval == 0x23) {
		// System is Little-Endian
		tmpval = cpio_head->h_filesize.hwds.lw16;
		cpio_head->h_filesize.hwds.lw16 = cpio_head->h_filesize.hwds.hi16;
		cpio_head->h_filesize.hwds.hi16 = tmpval;
		tmpval = cpio_head->h_mtime.hwds.lw16;
		cpio_head->h_mtime.hwds.lw16 = cpio_head->h_mtime.hwds.hi16;
		cpio_head->h_mtime.hwds.hi16 = tmpval;
	}
	
	return;
}

void* get_cpio_entry_header(char* filename, void* cpio_archive, struct header_pwb_cpio* buffer) {
	char* ptr = cpio_archive;
	uint32_t entry_datasize = 0;
	//DEBUG_print("Begin CPIO Parse\n");
	do {
		ptr += entry_datasize;
		/*
		char buf[20];
		itoa((uintRL_t)ptr, buf, 20, -16, -8);
		DEBUG_print(buf);
		DEBUG_print("--");
		*/
		struct header_pwb_cpio cpio_head;
		memcpy(&cpio_head, ptr, sizeof(struct header_pwb_cpio));
		if (cpio_head.h_magic != 0x71C7 && cpio_head.h_magic != 0xC771) {
			/*
			itoa(cpio_head.h_magic, buf, 20, -16, -4);
			DEBUG_print("End CPIO Parse 1: 0x");
			DEBUG_print(buf);
			DEBUG_print("\n");
			*/
			return 0;
		}
		ptr += sizeof(struct header_pwb_cpio);
		read_entry(&cpio_head);
		uint32_t namesize = cpio_head.h_namesize;
		namesize += namesize & 0x1;
		/*
		DEBUG_print("CPIO Entry Filename: ");
		DEBUG_print(ptr);
		DEBUG_print("\n");
		*/
		if (strcmp(ptr, filename) == 0) {
			memcpy(buffer, &cpio_head, sizeof(struct header_pwb_cpio));
			//DEBUG_print("End CPIO Parse 0\n");
			return ptr + namesize;
		}
		uint32_t filesize = cpio_head.h_filesize.vl32;
		filesize += filesize & 0x1;
		entry_datasize = namesize + filesize;
	} while (strcmp(ptr, "TRAILER!!!"));
	
	//DEBUG_print("End CPIO Parse 2\n");
	return 0;
}

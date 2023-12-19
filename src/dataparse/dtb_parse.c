#include "dtb_parse.h"
#include <debug.h>
#include <string.h>

uint32_t dtb_parse_swap_endianess_32(uint32_t val) {
	uint32_t retval = 0;
	unsigned int bitwidth = 32;
	unsigned int j = bitwidth / 2;
	unsigned int k = j - 8;
	for (unsigned int i = 8; i < bitwidth; i += 16) {
		retval |= (val >> i) & (0xFFul << k);
		retval |= (val << i) & (0xFFul << j);
		k -= 8;
		j += 8;
	}
	return retval;
}

uint64_t dtb_parse_swap_endianess_64(uint64_t val) {
	uint64_t retval = 0;
	unsigned int bitwidth = 64;
	unsigned int j = bitwidth / 2;
	unsigned int k = j - 8;
	for (unsigned int i = 8; i < bitwidth; i += 16) {
		retval |= (val >> i) & (0xFFul << k);
		retval |= (val << i) & (0xFFul << j);
		k -= 8;
		j += 8;
	}
	return retval;
}

void dtb_parse_swap_endianess_32_ptr(uint32_t* ptr) {
	*ptr = dtb_parse_swap_endianess_32(*ptr);
	return;
}

void dtb_parse_swap_endianess_64_ptr(uint64_t* ptr) {
	*ptr = dtb_parse_swap_endianess_64(*ptr);
	return;
}

void dtb_parse(void* dtb_location, void** initrd_start, void** initrd_end) {
	uint32_t* ptr = 0;
	struct fdt_header dtb_header;
	memcpy(&dtb_header, dtb_location, sizeof(struct fdt_header));
	if        (dtb_header.magic == 0xD00DFEED) {
		// System is Big-Endian
	} else if (dtb_header.magic == 0xEDFE0DD0) {
		// System is Little-Endian
		ptr = (void*)&dtb_header;
		for (unsigned int i = 0; i < (sizeof(struct fdt_header) / sizeof(uint32_t)); i++) {
			dtb_parse_swap_endianess_32_ptr(ptr + i);
		}
	} else {
		return;
	}
	
	void* rsvmap     = dtb_location + dtb_header.off_mem_rsvmap;
	void* dt_struct  = dtb_location + dtb_header.off_dt_struct;
	void* dt_strings = dtb_location + dtb_header.off_dt_strings;
	
	{
		unsigned int j = 0;
		for (void* i = rsvmap; (void*)i < dt_struct; i += sizeof(struct fdt_reserve_entry)) {
			struct fdt_reserve_entry rsv_entry;
			memcpy(&rsv_entry, i, sizeof(struct fdt_reserve_entry));
			if (ptr) {
				dtb_parse_swap_endianess_64_ptr(&rsv_entry.address);
				dtb_parse_swap_endianess_64_ptr(&rsv_entry.size);
			}
			if (rsv_entry.address == 0 && rsv_entry.size == 0) {
				break;
			}
			j++;
		}
	}
	
	uintRL_t linux_initrd_start = 0;
	uintRL_t linux_initrd_end   = 0;
	
	{
		uint32_t tree_depth = 0;
		void* i = dt_struct;
		while ((void*)i < dt_strings && (void*)i < (dt_struct + dtb_header.size_dt_struct)) {
			uint32_t prop_token;
			memcpy(&prop_token, i, sizeof(uint32_t));
			if (ptr) {
				prop_token = dtb_parse_swap_endianess_32(prop_token);
			}
			if        (prop_token == FDT_END) {
				break;
			} else if (prop_token == FDT_END_NODE) {
				if (tree_depth > 0) {
					tree_depth--;
				}
				i += sizeof(uint32_t);
			} else if (prop_token == FDT_BEGIN_NODE) {
				tree_depth++;
				i += sizeof(uint32_t);
				uintRL_t addr = (uintRL_t)i;
				addr += strlen(i) + 1;
				addr -= (uintRL_t)dt_struct;
				if (addr & (sizeof(uint32_t) - 1)) {
					addr &= ~((uintRL_t)(sizeof(uint32_t) - 1));
					addr += sizeof(uint32_t);
				}
				addr += (uintRL_t)dt_struct;
				i = (void*)addr;
			} else if (prop_token == FDT_NOP) {
				i += sizeof(uint32_t);
			} else if (prop_token == FDT_PROP) {
				i += sizeof(uint32_t);
				struct ftd_struct_prop_data prop_data;
				memcpy(&prop_data, i, sizeof(struct ftd_struct_prop_data));
				i += sizeof(struct ftd_struct_prop_data);
				uint32_t len = prop_data.len;
				uint32_t nameoff = prop_data.nameoff;
				if (ptr) {
					len = dtb_parse_swap_endianess_32(len);
					nameoff = dtb_parse_swap_endianess_32(nameoff);
				}
				uint8_t* byte_ptr = i;
				if        (strcmp(dt_strings + nameoff, "linux,initrd-start") == 0) {
					for (uint32_t j = 0; j < len; j++) {
						linux_initrd_start <<= 8;
						linux_initrd_start |= byte_ptr[j];
					}
				} else if (strcmp(dt_strings + nameoff, "linux,initrd-end"  ) == 0) {
					for (uint32_t j = 0; j < len; j++) {
						linux_initrd_end <<= 8;
						linux_initrd_end |= byte_ptr[j];
					}
				}
				uintRL_t addr = (uintRL_t)i;
				addr += len;
				addr -= (uintRL_t)dt_struct;
				if (addr & (sizeof(uint32_t) - 1)) {
					addr &= ~((uintRL_t)(sizeof(uint32_t) - 1));
					addr += sizeof(uint32_t);
				}
				addr += (uintRL_t)dt_struct;
				i = (void*)addr;
			} else {
				i += sizeof(uint32_t);
			}
		}
	}
	
	if (initrd_start) {
		*initrd_start = (void*)linux_initrd_start;
	}
	if (initrd_end) {
		*initrd_end   = (void*)linux_initrd_end;
	}
	
	return;
}

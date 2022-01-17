#include "./dtb_parse.h"
#include "./debug.h"
#include "./../inc/string.h"

#define DEBUG_print(arg1) {} // Disable Debug Printing

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
	
	char buf[20];
	DEBUG_print("\n----FDT Header----\n");
	
	DEBUG_print("\t            dtb_header.magic: 0x");
	itoa(dtb_header.magic, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t        dtb_header.totalsize: 0x");
	itoa(dtb_header.totalsize, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t    dtb_header.off_dt_struct: 0x");
	itoa(dtb_header.off_dt_struct, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t   dtb_header.off_dt_strings: 0x");
	itoa(dtb_header.off_dt_strings, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t   dtb_header.off_mem_rsvmap: 0x");
	itoa(dtb_header.off_mem_rsvmap, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t          dtb_header.version: 0x");
	itoa(dtb_header.version, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\tdtb_header.last_comp_version: 0x");
	itoa(dtb_header.last_comp_version, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t  dtb_header.boot_cpuid_phys: 0x");
	itoa(dtb_header.boot_cpuid_phys, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t  dtb_header.size_dt_strings: 0x");
	itoa(dtb_header.size_dt_strings, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t   dtb_header.size_dt_struct: 0x");
	itoa(dtb_header.size_dt_struct, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	void* rsvmap     = dtb_location + dtb_header.off_mem_rsvmap;
	void* dt_struct  = dtb_location + dtb_header.off_dt_struct;
	void* dt_strings = dtb_location + dtb_header.off_dt_strings;
	
	DEBUG_print("\n----FDT Rsv Map----\n");
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
			
			DEBUG_print("\t");
			itoa(j, buf, 20, -10, -2);
			DEBUG_print(buf);
			DEBUG_print(" ");
			
			DEBUG_print("Addr: 0x");
			uint64_t addr = rsv_entry.address;
			uint32_t addr_hi = (addr >> 32) & 0xFFFFFFFF;
			uint32_t addr_lo = (addr >>  0) & 0xFFFFFFFF;
			
			itoa(addr_hi, buf, 20, -16, -8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(addr_lo, buf, 20, -16, -8);
			DEBUG_print(buf);
			DEBUG_print(" | ");
			
			uint64_t size = rsv_entry.size;
			uint32_t size_hi = (size >> 32) & 0xFFFFFFFF;
			uint32_t size_lo = (size >>  0) & 0xFFFFFFFF;
			DEBUG_print("Size: 0x");
			itoa(size_hi, buf, 20, -16, -8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(size_lo, buf, 20, -16, -8);
			DEBUG_print(buf);
			DEBUG_print("\n");
			
			j++;
		}
	}
	
	uintRL_t linux_initrd_start = 0;
	uintRL_t linux_initrd_end   = 0;
	
	DEBUG_print("\n----FDT DT Struct----\n");
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
				{
					for (uint32_t i = 0; i < tree_depth; i++) {
						DEBUG_print(" ");
					}
				}
				DEBUG_print("FDT_END\n");
				break;
			} else if (prop_token == FDT_END_NODE) {
				if (tree_depth > 0) {
					tree_depth--;
				} else {
					DEBUG_print("E->");
				}
				{
					for (uint32_t i = 0; i < tree_depth; i++) {
						DEBUG_print(" ");
					}
				}
				DEBUG_print("FDT_END_NODE\n");
				i += sizeof(uint32_t);
			} else if (prop_token == FDT_BEGIN_NODE) {
				{
					for (uint32_t i = 0; i < tree_depth; i++) {
						DEBUG_print(" ");
					}
				}
				DEBUG_print("FDT_BEGIN_NODE ");
				tree_depth++;
				i += sizeof(uint32_t);
				DEBUG_print(i);
				DEBUG_print("\n");
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
				{
					for (uint32_t i = 0; i < tree_depth; i++) {
						DEBUG_print(" ");
					}
				}
				DEBUG_print("FDT_NOP\n");
				i += sizeof(uint32_t);
			} else if (prop_token == FDT_PROP) {
				{
					for (uint32_t i = 0; i < tree_depth; i++) {
						DEBUG_print(" ");
					}
				}
				DEBUG_print("FDT_PROP [len: ");
				i += sizeof(uint32_t);
				//struct ftd_struct_prop_data* prop_data = (void*)i;
				struct ftd_struct_prop_data prop_data;
				memcpy(&prop_data, i, sizeof(struct ftd_struct_prop_data));
				i += sizeof(struct ftd_struct_prop_data);
				uint32_t len = prop_data.len;
				uint32_t nameoff = prop_data.nameoff;
				if (ptr) {
					len = dtb_parse_swap_endianess_32(len);
					nameoff = dtb_parse_swap_endianess_32(nameoff);
				}
				itoa(len, buf, 20, -10, -4);
				DEBUG_print(buf);
				DEBUG_print(", nameoff: ");
				itoa(nameoff, buf, 20, -16, -8);
				DEBUG_print(buf);
				DEBUG_print("] '");
				DEBUG_print(dt_strings + nameoff);
				DEBUG_print("' =");
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
				for (uint32_t j = 0; j < len; j++) {
					DEBUG_print(" ");
					itoa(byte_ptr[j], buf, 20, -16, -2);
					DEBUG_print(buf);
				}
				DEBUG_print("\n");
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
				DEBUG_print("TOKEN Invalid: prop_token = ");
				itoa(prop_token, buf, 20, -16, -8);
				DEBUG_print(buf);
				DEBUG_print("\n");
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

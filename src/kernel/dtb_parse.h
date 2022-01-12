#ifndef _insertion_dtb_parse_h
#define _insertion_dtb_parse_h

#include "./../inc/types.h"

struct fdt_header {
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_dt_struct;
	uint32_t off_dt_strings;
	uint32_t off_mem_rsvmap;
	uint32_t version;
	uint32_t last_comp_version;
	uint32_t boot_cpuid_phys;
	uint32_t size_dt_strings;
	uint32_t size_dt_struct;
};

struct fdt_reserve_entry {
	uint64_t address;
	uint64_t size;
};

#define FDT_BEGIN_NODE ((uint32_t)0x00000001)
#define FDT_END_NODE   ((uint32_t)0x00000002)
#define FDT_PROP       ((uint32_t)0x00000003)
#define FDT_NOP        ((uint32_t)0x00000004)
#define FDT_END        ((uint32_t)0x00000009)

struct ftd_struct_prop_data {
	uint32_t len;
	uint32_t nameoff;
};

void dtb_parse(void* dtb_location, void** initrd_start, void** initrd_end);

#endif

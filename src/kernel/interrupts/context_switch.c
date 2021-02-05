#include "context_switch.h"
#include "./../../inc/string.h"
#include "./../memalloc.h"

struct __attribute__ ((packed)) ELF_FileHeader32 {
	char MagicNumber[4];
	uint8_t Bitsize;
	uint8_t Endianness;
	uint8_t ELF_Version1;
	uint8_t OS_ABI;
	uint8_t ABI_Version;
	uint8_t Padding1[7];
	uint16_t Type;
	uint16_t Machine;
	uint32_t ELF_Version2;
	uint32_t EntryPoint;
	uint32_t ProgramHeader;
	uint32_t SectionHeader;
	uint32_t Flags;
	uint16_t FileHeaderSize;
	uint16_t ProgramHeaderEntrySize;
	uint16_t ProgramHeaderEntryCount;
	uint16_t SectionHeaderEntrySize;
	uint16_t SectionHeaderEntryCount;
	uint16_t SectionHeaderEntryIndex_SectionNames;
};

struct __attribute__ ((packed)) ELF_ProgramHeader32 {
	uint32_t Type;
	uint32_t Offset;
	uint32_t VirtualAddress;
	uint32_t PhysicalAddress;
	uint32_t SizeInFile;
	uint32_t SizeInMemory;
	uint32_t Flags32;
	uint32_t Align;
};

struct __attribute__ ((packed)) ELF_SectionHeader32 {
	uint32_t Name;
	uint32_t Type;
	uint32_t Flags;
	uint32_t Address;
	uint32_t Offset;
	uint32_t Size;
	uint32_t Link;
	uint32_t Info;
	uint32_t AddressAlign;
	uint32_t EntrySize;
};

/*
struct __attribute__ ((packed)) ELF_SymtabEntry {
	uint32_t st_name;
	uint32_t st_value;
	uint32_t st_size;
	uint8_t st_info;
	uint8_t st_other;
	uint16_t st_shndx;
};
*/

// This function checks some header data and verifies memory offsets are within the filesize
uint32_t sanitize_elf(char* elf_data, size_t elf_data_size) {
	if (elf_data_size < sizeof(struct ELF_FileHeader32)) {
		// The provided data size is too small to even fit an ELF header
		return 1;
	}
	struct ELF_FileHeader32* ElfHeader = (struct ELF_FileHeader32*)elf_data;
	if (ElfHeader->MagicNumber[0] != 0x7F || ElfHeader->MagicNumber[1] != 'E' || ElfHeader->MagicNumber[2] != 'L' || ElfHeader->MagicNumber[3] != 'F') {
		// Invalid Magic Number
		return 2;
	}
	if (ElfHeader->Bitsize != 1) {
		// Incompatible Bit Width
		return 3;
	}
	if (ElfHeader->Endianness != 1) {
		// Incompatible Endianness
		return 4;
	}
	if (ElfHeader->ELF_Version1 != 1) {
		// Incompatible ELF Version
		return 5;
	}
	if (ElfHeader->ProgramHeaderEntrySize != sizeof(struct ELF_ProgramHeader32)) {
		// Protect against strange entry sizes and overflow attacks
		return 6;
	}
	if (ElfHeader->ProgramHeaderEntryCount > 100) {
		// Protect against overflow attacks by limiting unusually high program entry counts
		return 7;
	}
	if (ElfHeader->ProgramHeader > elf_data_size || \
		 ElfHeader->ProgramHeader + ElfHeader->ProgramHeaderEntrySize * ElfHeader->ProgramHeaderEntryCount > elf_data_size) {
		return 8;
	}
	if (ElfHeader->SectionHeaderEntrySize != sizeof(struct ELF_SectionHeader32)) {
		// Protect against strange entry sizes and overflow attacks
		return 9;
	}
	if (ElfHeader->SectionHeaderEntryCount > 100) {
		// Protect against overflow attacks by limiting unusually high section entry counts
		return 10;
	}
	if (ElfHeader->SectionHeader > elf_data_size || \
		 ElfHeader->SectionHeader + ElfHeader->SectionHeaderEntrySize * ElfHeader->SectionHeaderEntryCount > elf_data_size) {
		return 11;
	}
	if (ElfHeader->SectionHeaderEntryIndex_SectionNames >= ElfHeader->SectionHeaderEntryCount) {
		return 12;
	}
	uint32_t i;
	//uint32_t no_shstrndx = 1;
	i = 0;
	struct ELF_SectionHeader32* section = (struct ELF_SectionHeader32*)(elf_data + ElfHeader->SectionHeader);
	while (i < ElfHeader->SectionHeaderEntryCount) {
		if (section[i].Offset >= elf_data_size) {
			return 13;
		}
		if (section[i].Offset + section[i].Size >= elf_data_size) {
			return 14;
		}
		//if (i == ElfHeader->SectionHeaderEntryIndex_SectionNames) {
		//	no_shstrndx = 0;
		//}
		i++;
	}
	//if (no_shstrndx) {
	//	return 15;
	//}
	return 0;
}

uint32_t load_context_from_elf_data(char* elf_data, size_t elf_data_size, CPU_Context* context_ptr, void** mem_base, void** mem_top) {
	uint32_t ret_code = sanitize_elf(elf_data, elf_data_size);
	if (ret_code != 0) {
		return ret_code | 0x100;
	}
	struct ELF_FileHeader32* ElfHeader = (struct ELF_FileHeader32*)elf_data;
	if (ElfHeader->ProgramHeaderEntryCount == 0) {
		return 2;
	}
	struct ELF_ProgramHeader32* ElfProgram = (struct ELF_ProgramHeader32*)(elf_data + ElfHeader->ProgramHeader);
	//uintRL_t program_offset = ElfProgram[0].Offset;
	uintRL_t program_vm_location = ElfProgram[0].VirtualAddress;
	uintRL_t program_size = ElfProgram[0].SizeInFile;
	void* program = kmalloc(program_size + 0x500);
	if (program == 0) {
		return 3;
	}
	context_ptr->program_memory = (uintRL_t)program;
	if (ElfHeader->EntryPoint < program_vm_location || ElfHeader->EntryPoint >= program_vm_location + program_size) {
		kfree(program);
		return 4;
	}
	context_ptr->regs[0] = (uintRL_t)(ElfHeader->EntryPoint - program_vm_location + program);
	context_ptr->regs[2] = ((uintRL_t)(program + program_size + 0x500)) & ~((uintRL_t)0xF);
	if (mem_base != 0) {
		*mem_base = (void*)(program);
	}
	if (mem_top != 0) {
		*mem_top = (void*)(program + program_size + 0x500);
	}
	struct ELF_SectionHeader32* ElfSection = (struct ELF_SectionHeader32*)(elf_data + ElfHeader->SectionHeader);
	uint32_t i = 0;
	while (i < ElfHeader->SectionHeaderEntryCount) {
		if (ElfSection[i].Size != 0) {
			if (ElfSection[i].Address >= program_vm_location && ElfSection[i].Address - program_vm_location + ElfSection[i].Size <= program_size) {
				memcpy(program + (ElfSection[i].Address - program_vm_location), elf_data + ElfSection[i].Offset, ElfSection[i].Size);
				void* section_names_start = elf_data + ElfSection[ElfHeader->SectionHeaderEntryIndex_SectionNames].Offset;
				void* section_names_end = elf_data + ElfSection[ElfHeader->SectionHeaderEntryIndex_SectionNames].Offset + ElfSection[ElfHeader->SectionHeaderEntryIndex_SectionNames].Size;
				if (section_names_start + ElfSection[i].Name + 5 <= section_names_end) {
					if (strncmp(".got", section_names_start + ElfSection[i].Name, 5) == 0) {
						void** ptrs = program + (ElfSection[i].Address - program_vm_location);
						uint32_t j = 0;
						while (j * sizeof(void*) + sizeof(void*) <= ElfSection[i].Size) {
							if (ptrs[j] >= (void*)program_vm_location && ptrs[j] < (void*)(program_vm_location + program_size)) {
								ptrs[j] = ptrs[j] - program_vm_location + (uintRL_t)program;
							}
							j++;
						}
					}
				}
			}
		}
		i++;
	}
	return 0;
}

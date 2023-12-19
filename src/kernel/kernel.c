#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./drivers/uart.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./kernel.h"
#include "./cpio_parse.h"
#include "./dtb_parse.h"
#include "./kstart_entry.h"
#include "./thread_locking.h"
#include "./sbi_commands.h"
#include "./globals.h"
#include "./debug.h"

/** Expected value of info magic ('OSBI' ascii string in hex) */
#define FW_DYNAMIC_INFO_MAGIC_VALUE     0x4942534f
// 0x  01 23 45 67  89 AB CD EF
// 0x  EF CD AB 89  67 45 23 01

// 0x  4F 53 42 49  00 00 00 00
// 0x  00 00 00 00  49 42 53 4F

/** Maximum supported info version */
#define FW_DYNAMIC_INFO_VERSION         0x2

/** Possible next mode values */
#define FW_DYNAMIC_INFO_NEXT_MODE_U     0x0
#define FW_DYNAMIC_INFO_NEXT_MODE_S     0x1
#define FW_DYNAMIC_INFO_NEXT_MODE_M     0x3

struct fw_dynamic_info {
	/** Info magic */
	uintRL_t magic;
	/** Info version */
	uintRL_t version;
	/** Next booting stage address */
	uintRL_t next_addr;
	/** Next booting stage mode */
	uintRL_t next_mode;
	/** Options for OpenSBI library */
	uintRL_t options;
	/**
	 * Preferred boot HART id
	 *
	 * It is possible that the previous booting stage uses same link
	 * address as the FW_DYNAMIC firmware. In this case, the relocation
	 * lottery mechanism can potentially overwrite the previous booting
	 * stage while other HARTs are still running in the previous booting
	 * stage leading to boot-time crash. To avoid this boot-time crash,
	 * the previous booting stage can specify last HART that will jump
	 * to the FW_DYNAMIC firmware as the preferred boot HART.
	 *
	 * To avoid specifying a preferred boot HART, the previous booting
	 * stage can set it to -1UL which will force the FW_DYNAMIC firmware
	 * to use the relocation lottery mechanism.
	 */
	uintRL_t boot_hart;
};

extern char PROGAMIMAGE_START;
extern void* mem_block_end;
extern ksemaphore_t* sbi_hsm_locks;
extern volatile sint32_t* sbi_hsm_states;

ksemaphore_t* hart_command_que_locks;
uintRL_t init_reg_a0;
uintRL_t init_reg_a1;
uintRL_t init_reg_a2;
uintRL_t init_reg_a3;
uintRL_t init_reg_a4;
uintRL_t init_reg_a5;
uintRL_t kernel_load_to_point;
uintRL_t hart_context_count;
volatile CPU_Context* hart_contexts;
volatile CPU_Context* hart_contexts_exception;
volatile CPU_Context* hart_contexts_user;
volatile Hart_Command* hart_commands;

__thread uintRL_t mhartid;

void kinit(uintRL_t hartid) {
	kallocinit(&KHEAP_START, &KHEAP_START + 0x10000);
	//kalloc_pageinit(&KHEAP_START + 0x8000, &KHEAP_START + 0x8000 + 0x10000000);
	
	hart_commands = kmalloc(TOTAL_HART_COUNT * sizeof(Hart_Command));
	hart_command_que_locks = kmalloc(TOTAL_HART_COUNT * sizeof(ksemaphore_t));
	sbi_hsm_locks = kmalloc(TOTAL_HART_COUNT * sizeof(ksemaphore_t));
	sbi_hsm_states = kmalloc(TOTAL_HART_COUNT * sizeof(sint32_t));
	for (uintRL_t i = 0; i < TOTAL_HART_COUNT; i++) {
		hart_commands[i].command = 0;
		hart_commands[i].param0 = 0;
		hart_commands[i].param1 = 0;
		hart_commands[i].param2 = 0;
		hart_commands[i].param3 = 0;
		hart_commands[i].param4 = 0;
		hart_commands[i].param5 = 0;
		ksem_init(hart_command_que_locks + i);
		ksem_init(sbi_hsm_locks + i);
		sbi_hsm_states[i] = SBI_HSM_STOPPED;
	}
	
	// Note:
	//   &THI_START == &THI_tdata_START
	//   &THI_END == &THI_tbss_END
	//   The linker script should set them to be same.
	uintRL_t tls_size = (uintRL_t)(&THI_END) - (uintRL_t)(&THI_START);
	uintRL_t tls_init_size = (uintRL_t)(&THI_tdata_END) - (uintRL_t)(&THI_tdata_START);
	uintRL_t tls_null_size = (uintRL_t)(&THI_tbss_END) - (uintRL_t)(&THI_tbss_START);
	uintRL_t tls_null_offt = (uintRL_t)(&THI_tbss_START) - (uintRL_t)(&THI_START);
	hart_context_count = TOTAL_HART_COUNT * 3;
	hart_contexts = kmalloc(hart_context_count * sizeof(CPU_Context));
	hart_contexts_exception = hart_contexts + TOTAL_HART_COUNT;
	hart_contexts_user = hart_contexts_exception + TOTAL_HART_COUNT;
	
	// Setup the hart's context structs
	for (uintRL_t i = 0; i < TOTAL_HART_COUNT; i++) {
		void* ptr;
		ptr = kmalloc(tls_size);
		memcpy(ptr, &THI_tdata_START, tls_init_size);
		memset(ptr + tls_null_offt, 0, tls_null_size);
		clear_hart_context(hart_contexts + i);
		hart_contexts[i].context_id = i;
		hart_contexts[i].execution_mode = EM_M;
		hart_contexts[i].regs[REG_TP] = (uintRL_t)ptr;
		hart_contexts[i].regs[REG_SP] = (uintRL_t)kmalloc_stack(STACK_SIZE);
		// The special registers (sp and tp) in these harts will be set up
		// by the one-time interrupt handler that they run when we interrupt
		// them from their current spinning/halted state.
	}
	for (uintRL_t i = TOTAL_HART_COUNT; i < (TOTAL_HART_COUNT * 3); i++) {
		clear_hart_context(hart_contexts + i);
	}
	
	__asm__ __volatile__ ("csrw mscratch, %0" : : "r" (hart_contexts_exception + hartid) : "memory");
	__asm__ __volatile__ ("sfence.vma" : : : "memory");
	__asm__ __volatile__ ("fence.i" : : : "memory");
	
	// Start and wait for harts to initialize
	volatile uint32_t* clint_hart_msip_ctls = (void*)CLINT_BASE;
	for (uintRL_t i = 0; i < TOTAL_HART_COUNT; i++) {
		clint_hart_msip_ctls[i] = 0x1;
	}
	for (uintRL_t i = 0; i < TOTAL_HART_COUNT; i++) {
		while (clint_hart_msip_ctls[i]) {}
	}
	
	return;
}

void wait_by_spin() {
	for (volatile unsigned int i = 0; i < 200000000; i++) {}
	return;
}

void kmain() {
	printm("Hello, World!\n\n");
	
	unsigned char* ptr = (void*)UART0_BASE;
	printm("RHR: 0x%02X\n", ptr[0]);
	printm("IER: 0x%02X\n", ptr[1]);
	printm("ISR: 0x%02X\n", ptr[2]);
	printm("LCR: 0x%02X\n", ptr[3]);
	printm("MCR: 0x%02X\n", ptr[4]);
	printm("LSR: 0x%02X\n", ptr[5]);
	printm("MSR: 0x%02X\n", ptr[6]);
	printm("SPR: 0x%02X\n", ptr[7]);
	unsigned char vls[2];
	ptr[3] |=  0x80;
	vls[0] = ptr[0];
	vls[1] = ptr[1];
	ptr[3] &= ~0x80;
	printm("DLL: 0x%02X\n", vls[0]);
	printm("DLM: 0x%02X\n", vls[1]);
	printm("\n");
	
	uintRL_t mtvec;
	__asm__ __volatile__ ("csrr %0, mtvec" : "=r" (mtvec));
	printm("mtvec: %08X\n", mtvec);
	
	init_reg_a0 = 0;
	init_reg_a1 = 0;
	init_reg_a2 = 0;
	init_reg_a3 = 0;
	init_reg_a4 = 0;
	init_reg_a5 = 0;
	
	printm("             Register a0: 0x%08lX\n", init_reg_a0);
	printm("             Register a1: 0x%08lX\n", init_reg_a1);
	printm("             Register a2: 0x%08lX\n", init_reg_a2);
	printm("             Register a3: 0x%08lX\n", init_reg_a3);
	printm("             Register a4: 0x%08lX\n", init_reg_a4);
	printm("             Register a5: 0x%08lX\n", init_reg_a5);
	printm("\n");
	
	struct __attribute__((__packed__)) kernel_boot_header {
		uint32_t code0;
		uint32_t code1;
		uint32_t text_offset_a;
		uint32_t text_offset_b;
		uint32_t image_size_a;
		uint32_t image_size_b;
		uint32_t flags_a;
		uint32_t flags_b;
		uint16_t version_minor;
		uint16_t version_major;
		uint32_t res1;
		uint32_t res2_a;
		uint32_t res2_b;
		uint32_t magic_a;
		uint32_t magic_b;
		uint32_t magic2;
		uint32_t res3;
	};
	
	// Start: Locate Kernel
	struct kernel_boot_header* unloaded_kernel_ptr = 0;
	if (init_reg_a2) {
		struct fw_dynamic_info fw_dyninfo;
		memcpy(&fw_dyninfo, (void*)init_reg_a2, sizeof(uintRL_t));
		if (fw_dyninfo.magic == FW_DYNAMIC_INFO_MAGIC_VALUE) {
			memcpy(&fw_dyninfo, (void*)init_reg_a2, sizeof(struct fw_dynamic_info));
			unloaded_kernel_ptr = (void*)(fw_dyninfo.next_addr);
		}
	}
	struct header_pwb_cpio cpio_kernel_entry_header;
	memset(&cpio_kernel_entry_header, 0, sizeof(struct header_pwb_cpio));
	cpio_kernel_entry_header.h_magic = 0;
	if (unloaded_kernel_ptr == 0) {
		void* ptr = (void*)(0x20000000);
		unloaded_kernel_ptr = get_cpio_entry_header("kernel.bin", ptr, &cpio_kernel_entry_header);
		//unloaded_kernel_ptr = get_cpio_entry_header("prog-emu.elf.strip.bin", ptr, &cpio_kernel_entry_header);
	}
	if (unloaded_kernel_ptr == 0) {
		printm("Could Not Locate Kernel: Halting\n");
		idle_loop();
	}
	// End: Locate Kernel
	
	// Start: Locate DTB
	void* unloaded_dtb_ptr = 0;
	struct header_pwb_cpio cpio_dtb_entry_header;
	memset(&cpio_dtb_entry_header, 0, sizeof(struct header_pwb_cpio));
	if (init_reg_a1) {
		unloaded_dtb_ptr = (void*)(init_reg_a1);
	}
	unsigned int dtb_from_qemu;
	if (unloaded_dtb_ptr == 0) {
		void* ptr = (void*)(0x20000000);
		unloaded_dtb_ptr = get_cpio_entry_header("echelon_emu_v0.1.1.dtb", ptr, &cpio_dtb_entry_header);
		dtb_from_qemu = 0;
	} else {
		dtb_from_qemu = 1;
	}
	if (unloaded_dtb_ptr == 0) {
		printm("Could Not Locate DTB Image: Halting\n");
		idle_loop();
	}
	// End: Locate DTB
	
	// Start: Locate Initramfs
	void* unloaded_initramfs_ptr = 0;
	struct header_pwb_cpio cpio_initramfs_entry_header;
	memset(&cpio_initramfs_entry_header, 0, sizeof(struct header_pwb_cpio));
	if (dtb_from_qemu) {
		dtb_parse(unloaded_dtb_ptr, &unloaded_initramfs_ptr, 0);
	}
	if (unloaded_initramfs_ptr == 0) {
		void* ptr = (void*)(0x20000000);
		unloaded_initramfs_ptr = get_cpio_entry_header("initramfs.cpio.gz", ptr, &cpio_initramfs_entry_header);
	}
	if (unloaded_initramfs_ptr == 0) {
		printm("Could Not Locate Initramfs: Halting\n");
		idle_loop();
	}
	// End: Locate Initramfs
	
	struct kernel_boot_header tmp_kernel_header;
	memcpy(&tmp_kernel_header, unloaded_kernel_ptr, sizeof(struct kernel_boot_header));
	
	uintRL_t kernel_header_text_offset = tmp_kernel_header.text_offset_b;
	kernel_header_text_offset <<= 32;
	kernel_header_text_offset |= tmp_kernel_header.text_offset_a;
	uintRL_t kernel_header_image_size = tmp_kernel_header.image_size_b;
	kernel_header_image_size <<= 32;
	kernel_header_image_size |= tmp_kernel_header.image_size_a;
	
	printm("    kernel_header_text_offset: 0x%08lX\n", kernel_header_text_offset);
	printm("     kernel_header_image_size: 0x%08lX\n", kernel_header_image_size);
	
	kernel_load_to_point = (uintRL_t)&PROGAMIMAGE_START; // 0x8000_0000
	kernel_load_to_point += kernel_header_text_offset;
	if (kernel_load_to_point & 0x001FFFFF) {
		kernel_load_to_point &= ~((uintRL_t)0x001FFFFF);
		kernel_load_to_point += 0x00200000; // 0x0020_0000
	}
	while (kernel_load_to_point < ((uintRL_t)mem_block_end)) {
		kernel_load_to_point += 0x00200000; // 0x0020_0000
	}
	printm(" Loading Kernel Fr @ 0x%08lX\n", (uintRL_t)unloaded_kernel_ptr);
	printm(" Loading Kernel To @ 0x%08lX\n", kernel_load_to_point);
	
	if (cpio_kernel_entry_header.h_magic != 0) {
		printm("Starting [%d]...", cpio_kernel_entry_header.h_filesize.vl32);
		memcpy((void*)kernel_load_to_point, unloaded_kernel_ptr, cpio_kernel_entry_header.h_filesize.vl32);
		printm("Done\n");
	} else {
		memcpy((void*)kernel_load_to_point, unloaded_kernel_ptr, kernel_header_image_size);
	}
	
	uintRL_t dtb_load_to_point;
	if (cpio_dtb_entry_header.h_magic != 0) {
		dtb_load_to_point = kernel_load_to_point + kernel_header_image_size + 0x10;
	} else {
		dtb_load_to_point = init_reg_a1;
	}
	dtb_load_to_point &= ~((uintRL_t)0xF);
	printm("       Loading DTB @ 0x%08lX\n", dtb_load_to_point);
	if (cpio_dtb_entry_header.h_magic != 0) {
		printm("Starting [%d]...", cpio_dtb_entry_header.h_filesize.vl32);
		memcpy((void*)dtb_load_to_point, unloaded_dtb_ptr, cpio_dtb_entry_header.h_filesize.vl32);
		printm("Done\n");
	}
	
	uintRL_t initramfs_load_to_point;
	if (cpio_initramfs_entry_header.h_magic != 0) {
		initramfs_load_to_point = 0x86000000;
	} else {
		initramfs_load_to_point = (uintRL_t)unloaded_initramfs_ptr;
	}
	printm(" Loading Initramfs @ 0x%08lX\n", initramfs_load_to_point);
	if (cpio_initramfs_entry_header.h_magic != 0) {
		printm("Starting [%d]...", cpio_initramfs_entry_header.h_filesize.vl32);
		memcpy((void*)initramfs_load_to_point, unloaded_initramfs_ptr, cpio_initramfs_entry_header.h_filesize.vl32);
		printm("Done\n");
	}
	
	printm("\n");
	
	{
		unsigned char* ptr = (void*)dtb_load_to_point;
		printm("DTB - First 16 Bytes\n  ");
		for (unsigned int i = 0; i <  8; i++) {
			printm(" %02X", ptr[i]);
		}
		printm("  ");
		for (unsigned int i = 8; i < 16; i++) {
			printm(" %02X", ptr[i]);
		}
		printm("\n\n");
	}
	{
		unsigned char* ptr = (void*)initramfs_load_to_point;
		printm("IRF - First 16 Bytes\n  ");
		for (unsigned int i = 0; i <  8; i++) {
			printm(" %02X", ptr[i]);
		}
		printm("  ");
		for (unsigned int i = 8; i < 16; i++) {
			printm(" %02X", ptr[i]);
		}
		printm("\n\n");
	}
	
	uintRL_t hart_to_start = 0;
	
	printm("--Start Hart--\n");
	Hart_Command command;
	command.command = HARTCMD_STARTHART;
	command.param0 = hart_to_start;
	command.param1 = kernel_load_to_point;
	command.param2 = dtb_load_to_point;
	send_hart_command_que(hart_to_start, &command);
	
	return;
}

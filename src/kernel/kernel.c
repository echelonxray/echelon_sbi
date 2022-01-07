#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./drivers/uart.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./kernel.h"
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

__thread uintRL_t mhartid;
ksemaphore_t* hart_command_que_locks;
uintRL_t init_reg_a0;
uintRL_t init_reg_a1;
uintRL_t init_reg_a2;
uintRL_t init_reg_a3;
uintRL_t init_reg_a4;
uintRL_t init_reg_a5;
uintRL_t load_point;
uintRL_t hart_context_count;
volatile CPU_Context* hart_contexts;
volatile CPU_Context* hart_contexts_exception;
volatile CPU_Context* hart_contexts_user;
volatile Hart_Command* hart_commands;

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
	volatile uint32_t* ctrl_reg;
	
	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;
	
	DEBUG_print("Hello, World!\n");
	
	//return;
	
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
	
	char buf[20];
	
	unsigned char* byte = (void*)(init_reg_a2);
	DEBUG_print("0x  ");
	for (uintRL_t i = 0; i < 8; i++) {
		char buf[20];
		itoa(byte[i], buf, 20, -16, 2);
		DEBUG_print(buf);
		DEBUG_print(" ");
	}
	for (uintRL_t i = 8; i < 16; i++) {
		DEBUG_print(" ");
		char buf[20];
		itoa(byte[i], buf, 20, -16, 2);
		DEBUG_print(buf);
	}
	DEBUG_print("\n");
	
	DEBUG_print("\n");
	struct kernel_boot_header* bkh = 0;
	if (init_reg_a2) {
		struct fw_dynamic_info fw_dyninfo;
		memcpy(&fw_dyninfo, (void*)init_reg_a2, sizeof(uintRL_t));
		if (fw_dyninfo.magic == FW_DYNAMIC_INFO_MAGIC_VALUE) {
			memcpy(&fw_dyninfo, (void*)init_reg_a2, sizeof(struct fw_dynamic_info));
			bkh = (void*)(fw_dyninfo.next_addr);
		}
	}
	if (bkh == 0) {
		struct header_pwb_cpio {
			uint16_t h_magic;
			uint16_t h_dev;
			uint16_t h_ino;
			uint16_t h_mode;
			uint16_t h_uid;
			uint16_t h_gid;
			uint16_t h_nlink;
			uint16_t h_majmin;
			uint32_t h_mtime;
			uint16_t h_namesize;
			uint32_t h_filesize;
		};
		struct header_pwb_cpio bin_header;
	}
	DEBUG_print("\n");
	
	itoa(init_reg_a0 >> 32, buf, 20, -16, 8);
	DEBUG_print("             reg_a0: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(init_reg_a0 & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(init_reg_a1 >> 32, buf, 20, -16, 8);
	DEBUG_print("             reg_a1: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(init_reg_a1 & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(init_reg_a2 >> 32, buf, 20, -16, 8);
	DEBUG_print("             reg_a2: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(init_reg_a2 & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(init_reg_a3 >> 32, buf, 20, -16, 8);
	DEBUG_print("             reg_a3: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(init_reg_a3 & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(init_reg_a4 >> 32, buf, 20, -16, 8);
	DEBUG_print("             reg_a4: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(init_reg_a4 & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(init_reg_a5 >> 32, buf, 20, -16, 8);
	DEBUG_print("             reg_a5: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(init_reg_a5 & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	struct fw_dynamic_info* fw_dyninfo = (void*)(init_reg_a2);
	itoa(fw_dyninfo->magic >> 32, buf, 20, -16, 8);
	DEBUG_print("              magic: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(fw_dyninfo->magic & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa((uintRL_t)bkh >> 32, buf, 20, -16, 8);
	DEBUG_print("     kernel_locatio: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa((uintRL_t)bkh & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\n");
	
	itoa(bkh->text_offset_b, buf, 20, -16, 8);
	DEBUG_print("     bkh.text_offse: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(bkh->text_offset_a, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(bkh->image_size_b, buf, 20, -16, 8);
	DEBUG_print("     bkh.image_size: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(bkh->image_size_a, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	uintRL_t text_offset = bkh->text_offset_b;
	text_offset <<= 32;
	text_offset |= bkh->text_offset_a;
	uintRL_t image_size = bkh->image_size_b;
	image_size <<= 32;
	image_size |= bkh->image_size_a;
	
	/*
	struct kernel_boot_header bkh;
	// TODO: Locate kernel
	// TODO: Load Kernel Header into "bkh"
	
	uintRL_t text_offset = bkh.text_offset_b;
	text_offset <<= 32;
	text_offset |= bkh.text_offset_a;
	uintRL_t image_size = bkh.image_size_b;
	image_size <<= 32;
	image_size |= bkh.image_size_a;
	*/
	
	load_point = (uintRL_t)&PROGAMIMAGE_START; // 0x8000_0000
	load_point += text_offset;
	if (load_point & 0x001FFFFF) {
		load_point &= ~((uintRL_t)0x001FFFFF);
		load_point += 0x00200000; // 0x0020_0000
	}
	while (load_point < ((uintRL_t)mem_block_end)) {
		load_point += 0x00200000; // 0x0020_0000
	}
	
	itoa((load_point >> 32) & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print("    Loading Kernel @ 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa((load_point >>  0) & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	// TODO: Load Kernel into main memory per the location in load_point
	
	itoa((init_reg_a1 >> 32) & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print("       Loading DTB @ 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa((init_reg_a1 >>  0) & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa((0 >> 32) & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(" Loading Initramfs @ 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa((0 >>  0) & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\n");
	
	memcpy((void*)load_point, bkh, image_size);
	
	DEBUG_print("--Start Hart--\n");
	Hart_Command command;
	command.command = HARTCMD_STARTHART;
	command.param0 = 1;
	command.param1 = load_point;
	command.param2 = init_reg_a1;
	send_hart_command_que(1, &command);
	
	return;
}

#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./drivers/uart.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./kernel.h"
#include "./thread_locking.h"
#include "./sbi_commands.h"
#include "./globals.h"
#include "./debug.h"

extern void* mem_block_end;

ksemaphore_t* hart_command_que_locks;
extern ksemaphore_t* sbi_hsm_locks;
extern volatile sint32_t* sbi_hsm_states;
uintRL_t dtb_location_a0;
uintRL_t dtb_location_a1;
uintRL_t dtb_location_a2;
uintRL_t dtb_location_a3;
uintRL_t hart_context_count;
volatile CPU_Context* hart_contexts;
volatile Hart_Command* hart_commands;
uintRL_t load_point;

__thread uintRL_t mhartid;

void kinit() {
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
	hart_context_count = (TOTAL_HART_COUNT + (USE_HART_COUNT * 1));
	hart_contexts = kmalloc(hart_context_count * sizeof(CPU_Context));
	
	// Setup this hart's (Hart 0) context struct
	void* ptr;
	ptr = kmalloc(tls_size);
	memcpy(ptr, &THI_tdata_START, tls_init_size);
	memset(ptr + tls_null_offt, 0, tls_null_size);
	clear_hart_context(hart_contexts + 0);
	hart_contexts[0].context_id = 0;
	hart_contexts[0].execution_mode = EM_M;
	hart_contexts[0].regs[REG_TP] = (uintRL_t)ptr;
	hart_contexts[0].regs[REG_SP] = (uintRL_t)&KISTACK_TOP;
	// Now that we actually have TLS memory allocated and setup for this
	// hart(M-Mode thread), set the Thread Pointer to it so that we can
	// actually use it.
	__asm__ __volatile__ ("mv tp, %0" : : "r" (ptr) : "memory");
	mhartid = 0;
	
	// Setup the other hart's context structs
	for (uintRL_t i = 1; i < TOTAL_HART_COUNT; i++) {
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
	
	// Start and wait for other harts to initialize
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	for (uintRL_t i = 1; i < TOTAL_HART_COUNT; i++) {
		clint_hart_msip_ctls[i] = 0x1;
	}
	for (uintRL_t i = 1; i < TOTAL_HART_COUNT; i++) {
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
	DEBUG_print("\n");
	
	struct __attribute__((__packed__)) kboot_header {
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
	
	struct kboot_header* bkh = (void*)(0x20000000);
	char buf[20];
	itoa(bkh->text_offset_b, buf, 20, -16, 8);
	DEBUG_print("bkh.text_offse: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(bkh->text_offset_a, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(bkh->image_size_b, buf, 20, -16, 8);
	DEBUG_print("bkh.image_size: 0x");
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
	
	load_point = 0x80000000; // 0x8000_0000
	load_point += text_offset;
	
	while (load_point < ((uintRL_t)mem_block_end)) {
		load_point += 0x00200000; // 0x0020_0000
	}
	
	itoa(load_point >> 32, buf, 20, -16, 8);
	DEBUG_print("    load_point: 0x");
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(load_point & 0xFFFFFFFF, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\n");
	
	/*
	DEBUG_print("dtb_location_a0: 0x");
	itoa(dtb_location_a0, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("dtb_location_a1: 0x");
	itoa(dtb_location_a1, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("dtb_location_a2: 0x");
	itoa(dtb_location_a2, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("dtb_location_a3: 0x");
	itoa(dtb_location_a3, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\n");
	
	DEBUG_print("----Hex Dump of 0x");
	itoa(dtb_location_a1, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print(" for ");
	itoa(8192, buf, 20, -10, 0);
	DEBUG_print(buf);
	DEBUG_print(" bytes:----");
	unsigned char* byte = (void*)(dtb_location_a1);
	for (uintRL_t i = 0; i < 8192; i++) {
		if        (i % 16 == 0) {
			DEBUG_print("\n0x");
			itoa(i, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print(":  ");
		} else if (i % 8 == 0) {
			DEBUG_print("  ");
		} else {
			DEBUG_print(" ");
		}
		itoa(byte[i], buf, 20, -16, 2);
		DEBUG_print(buf);
	}
	DEBUG_print("\n");
	
	DEBUG_print("----Hex Dump of 0x");
	itoa(dtb_location_a2, buf, 20, -16, 8);
	DEBUG_print(buf);
	DEBUG_print(" for ");
	itoa(0x50, buf, 20, -10, 0);
	DEBUG_print(buf);
	DEBUG_print(" bytes:----");
	byte = (void*)(dtb_location_a2);
	for (uintRL_t i = 0; i < 0x50; i++) {
		if        (i % 16 == 0) {
			DEBUG_print("\n0x");
			itoa(i, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print(":  ");
		} else if (i % 8 == 0) {
			DEBUG_print("  ");
		} else {
			DEBUG_print(" ");
		}
		itoa(byte[i], buf, 20, -16, 2);
		DEBUG_print(buf);
	}
	DEBUG_print("\n");
	*/
	
	memcpy((void*)load_point, (void*)0x20000000, image_size);
	
	clear_hart_context(hart_contexts + TOTAL_HART_COUNT + 0);
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_A0] = 1;
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_A1] = dtb_location_a1;
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_A2] = dtb_location_a2;
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_A3] = dtb_location_a3;
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_PC] = load_point;
	hart_contexts[TOTAL_HART_COUNT + 0].context_id = TOTAL_HART_COUNT + 0;
	hart_contexts[TOTAL_HART_COUNT + 0].execution_mode = EM_S;
	
	Hart_Command command;
	
	// Allow all memory access
	// QEMU will fail when switching to Supervisor mode of no PMP rules are set
	for (uintRL_t i = 1; i < 5; i++) {
		command.command = HARTCMD_SETPMPADDR;
		command.param1  = 0;
		command.param0  = 0x0000000080000000; // 0x0000_0000_8000_0000
		send_hart_command_blk(i, &command);
		command.command = HARTCMD_SETPMPADDR;
		command.param1  = 1;
		command.param0  = load_point;
		send_hart_command_blk(i, &command);
		command.command = HARTCMD_SETPMPADDR;
		command.param1  = 2;
		command.param0  = 0x003FFFFFFFFFFFFF; // 0x003F_FFFF_FFFF_FFFF
		send_hart_command_blk(i, &command);
		
		command.command = HARTCMD_SETPMPCFG;
		command.param1  = 0;
		command.param0  = 0;
		command.param0 |= (0x0F << 16) | (0x08 <<  8) | (0x0F <<  0);
		send_hart_command_blk(i, &command);
		
		command.command = HARTCMD_SETSATP;
		command.param0  = 0;
		send_hart_command_blk(i, &command);
		
		command.command = HARTCMD_GETSSTATUS;
		send_hart_command_blk(i, &command);
		command.param0 &= ~((uintRL_t)0x2);
		command.command = HARTCMD_SETSSTATUS;
		send_hart_command_blk(i, &command);
	}
	
	command.command = HARTCMD_SETEXCEPTIONDELEGATION;
	command.param0  = 0;
	command.param0 |= (1 <<  0) | (1 <<  1) | (1 <<  2) | (1 <<  3) | (1 <<  4) | (1 <<  5) | (1 <<  6);
	command.param0 |= (1 <<  7) | (1 <<  8) |                                     (1 << 12) | (1 << 13);
	command.param0 |=             (1 << 15);
	command.param0  = 0xb109;
	command.param0  = 0;
	send_hart_command_blk(1, &command);
	
	command.command = HARTCMD_SETINTERRUPTDELEGATION;
	command.param0  = 0;
	command.param0 |= (1 <<  0) | (1 <<  1) |                         (1 <<  4) | (1 <<  5)            ;
	command.param0 |=             (1 <<  8) | (1 <<  9);
	command.param0  = 0x0222;
	command.param0  = 0;
	send_hart_command_blk(1, &command);
	
	ksem_wait(sbi_hsm_locks + 1);
	sbi_hsm_states[1] = SBI_HSM_STARTED;
	ksem_post(sbi_hsm_locks + 1);
	
	command.command = HARTCMD_SWITCHCONTEXT;
	command.param0 = (uintRL_t)(hart_contexts + TOTAL_HART_COUNT + 0);
	send_hart_command_blk(1, &command);
	
	wait_by_spin();
	
	DEBUG_print("\n");
	DEBUG_print("Bye, World!\n");
	
	return;
}

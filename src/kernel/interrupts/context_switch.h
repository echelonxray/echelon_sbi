#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

typedef struct {
	uintRL_t context_id;
	uintRL_t execution_mode;
	uintRL_t reserved_0;
	uintRL_t regs[32];
} CPU_Context;

typedef struct {
	uintRL_t command;
	uintRL_t param0;
	uintRL_t param1;
	uintRL_t param2;
	uintRL_t param3;
	uintRL_t param4;
	uintRL_t param5;
} Hart_Command;

struct __attribute__((__packed__)) encoded_type_r {
	uint16_t rd: 5;
	uint16_t funct3: 3;
	uint16_t rs1: 5;
	uint16_t rs2: 5;
	uint16_t funct7: 12;
};
struct __attribute__((__packed__)) encoded_type_i {
	uint16_t rd: 5;
	uint16_t funct3: 3;
	uint16_t rs1: 5;
	uint16_t imm: 12;
};
struct __attribute__((__packed__)) encoded_type_s {
	uint16_t imm_a: 5;
	uint16_t funct3: 3;
	uint16_t rs1: 5;
	uint16_t rs2: 5;
	uint16_t imm_b: 7;
};
struct __attribute__((__packed__)) encoded_type_b {
	uint16_t imm_c: 1;
	uint16_t imm_a: 4;
	uint16_t funct3: 3;
	uint16_t rs1: 5;
	uint16_t rs2: 5;
	uint16_t imm_b: 6;
	uint16_t imm_d: 1;
};
struct __attribute__((__packed__)) encoded_type_u {
	uint16_t rd: 5;
	uint32_t imm: 20;
};
struct __attribute__((__packed__)) encoded_type_j {
	uint16_t rd: 5;
	uint16_t imm_a: 10;
	uint16_t imm_b: 1;
	uint16_t imm_c: 8;
	uint16_t imm_d: 1;
};

/*
typedef struct {
	uint8_t opcode: 7;
	union {
		struct encoded_type_r enc_r;
		struct encoded_type_i enc_i;
		struct encoded_type_s enc_s;
		struct encoded_type_b enc_b;
		struct encoded_type_u enc_u;
		struct encoded_type_j enc_j;
	} params;
} enc_inst;
*/
typedef struct {
	uintRL_t opcode;
	uintRL_t funct3;
	uintRL_t funct7;
	uintRL_t rd;
	uintRL_t rs1;
	uintRL_t rs2;
	uintRL_t imm;
} dec_inst;

uintRL_t walk_pts(uintRL_t location, uintRL_t csr_satp);
uintRL_t decode_instruction(uint32_t einst, dec_inst* dinst);
void interrupt_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value);
void exception_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value);
void interrupt_entry_handler();
void switch_context(volatile CPU_Context* cpu_context);
void clear_hart_context(volatile CPU_Context* hart_context);
void send_hart_command_que(uintRL_t hart_id, Hart_Command* command);
//void send_hart_command_lck(uintRL_t hart_id, Hart_Command* command);
void send_hart_command_blk(uintRL_t hart_id, Hart_Command* command);
void send_hart_command_ret(uintRL_t hart_id, Hart_Command* command);
void s_delegation_trampoline(volatile CPU_Context* cpu_context, uintRL_t pc_offset, uintRL_t stval);

#define HARTCMD_SWITCHCONTEXT 1
#define HARTCMD_STARTHART 14
#define HARTCMD_STOPHART 15 // Not a real implemented command
#define HARTCMD_SUSPENDHART 16 // Not a real implemented command
#define HARTCMD_REMOTE_FENCE_I 17
#define HARTCMD_REMOTE_SFENCE_VMA 18
#define HARTCMD_REMOTE_SFENCE_VMA_ASID 19
#define HARTCMD_SMODE_SOFTINT 20

#define EM_M 3
#define EM_S 1
#define EM_U 0

#endif

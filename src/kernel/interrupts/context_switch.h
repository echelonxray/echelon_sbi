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

void* hart_start_c_handler(uintRL_t hart_context_index, uintRL_t is_interrupt, uintRL_t cause_value);
void interrupt_c_handler(volatile CPU_Context* cpu_context, uintRL_t cpu_context_index, uintRL_t is_interrupt, uintRL_t cause_value);
void interrupt_entry_handler();
void switch_context(volatile CPU_Context* cpu_context);
void send_hart_command_que(uintRL_t hart_id, Hart_Command* command);
void send_hart_command_blk(uintRL_t hart_id, Hart_Command* command);
void send_hart_command_ret(uintRL_t hart_id, Hart_Command* command);

#define HARTCMD_SWITCHCONTEXT 1
#define HARTCMD_GETEXCEPTIONDELEGATION 2
#define HARTCMD_SETEXCEPTIONDELEGATION 3
#define HARTCMD_GETINTERRUPTDELEGATION 4
#define HARTCMD_SETINTERRUPTDELEGATION 5
#define HARTCMD_GETPMPADDR 6
#define HARTCMD_SETPMPADDR 7
#define HARTCMD_GETPMPCFG 8
#define HARTCMD_SETPMPCFG 9
#define HARTCMD_GETSATP 10
#define HARTCMD_SETSATP 11
#define HARTCMD_GETSSTATUS 12
#define HARTCMD_SETSSTATUS 13

#define EM_M 3
#define EM_S 1
#define EM_U 0

#endif

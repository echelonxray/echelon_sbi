#ifndef _insertion_kernel_interrupts_asm_header_h
#define _insertion_kernel_interrupts_asm_header_h

#include "./../inc/general_oper.h"

#ifndef __riscv
	#error "Not supported architecture"
#endif

#ifdef __riscv_32e
	#define REG_COUNT 16
#else
	#define REG_COUNT 32
#endif

#if   __riscv_xlen == 128
	#error "No XLEN Load/Store instruction defined for 128-Bit XLEN"
#elif __riscv_xlen == 64
	#define SX sd
	#define LX ld
	#define SAVE_REGS_S0 M_SAVE_REGS_64_S0
	#define LOAD_REGS_S0 M_LOAD_REGS_64_S0
	#define XLEN_BYTES 8
#else
	#define SX sw
	#define LX lw
	#define SAVE_REGS_S0 M_SAVE_REGS_32_S0
	#define LOAD_REGS_S0 M_LOAD_REGS_32_S0
	#define XLEN_BYTES 4
#endif

#define CT_R_ST (XLEN_BYTES * 3)
#define CT_SIZE (CT_R_ST + (XLEN_BYTES * REG_COUNT))

.altmacro
.macro M_LS_REGS_ELEM inst arg_reg ptr_reg offt
	\inst x\arg_reg, \offt(x\ptr_reg)
.endm

.macro M_LS_REGS_LIST inst arg_reg_start arg_reg_end ptr_reg
	.if \ptr_reg-\arg_reg_start
		M_LS_REGS_ELEM <\inst>, <\arg_reg_start>, <\ptr_reg>, %(CT_R_ST + (XLEN_BYTES * (\arg_reg_start)))
	.endif
	.if \arg_reg_end-\arg_reg_start
		M_LS_REGS_LIST <\inst>, %(\arg_reg_start + 1), <\arg_reg_end>, <\ptr_reg>
	.endif
.endm

.macro SAVE_REGS ptr_reg
#if REG_COUNT > 16 // Clang asm macro recursive limit is 20
	M_LS_REGS_LIST <SX>,  1,        %(16-1), <\ptr_reg>
	M_LS_REGS_LIST <SX>, 16, %(REG_COUNT-1), <\ptr_reg>
#else
	M_LS_REGS_LIST <SX>,  1, %(REG_COUNT-1), <\ptr_reg>
#endif
	.if \ptr_reg-1
		csrrw x1, mscratch, x\ptr_reg
		M_LS_REGS_ELEM <SX>, 1, <\ptr_reg>, %(CT_R_ST+(XLEN_BYTES*\ptr_reg))
	.else
		csrrw x2, mscratch, x\ptr_reg
		M_LS_REGS_ELEM <SX>, 2, <\ptr_reg>, %(CT_R_ST+(XLEN_BYTES*\ptr_reg))
	.endif
.endm

.macro LOAD_REGS ptr_reg
#if REG_COUNT > 16 // Clang asm macro recursive limit is 20
	M_LS_REGS_LIST <LX>,  1,        %(16-1), <\ptr_reg>
	M_LS_REGS_LIST <LX>, 16, %(REG_COUNT-1), <\ptr_reg>
#else
	M_LS_REGS_LIST <LX>,  1, %(REG_COUNT-1), <\ptr_reg>
#endif
	M_LS_REGS_ELEM <LX>, <\ptr_reg>, <\ptr_reg>, %(CT_R_ST+(XLEN_BYTES*\ptr_reg))
.endm

#endif

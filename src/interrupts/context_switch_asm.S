#include "context_switch_asm.h"
#include <inc/memmap.h>
#include <inc/csr.h>

.section .text

.globl interrupt_entry_handler
.globl hart_init_handler
.globl switch_context
.globl s_delegation_trampoline

.align 2, 0
hart_init_handler:
	sfence.vma
	fence.i
	
	# Save registers only if mscratch is already setup
	csrrw s0, mscratch, s0
	beq s0, zero, 2f
	SAVE_REGS REG_S0
	2:
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	csrr a0, mhartid
	
	1: auipc a1, %pcrel_hi(hart_contexts)
	LX a1, %pcrel_lo(1b)(a1)
	
	1: auipc a2, %pcrel_hi(hart_context_count)
	LX a2, %pcrel_lo(1b)(a2)
	
	li a3, 0
	li a5, 0
	2: # loop start
	bgeu a3, a2, 3f # Error: mhartid not found in array
	LX a4, (a1)
	beq a0, a4, 2f
	addi a1, a1, CT_SIZE
	addi a5, a5, CT_SIZE
	addi a3, a3, 1
	j 2b
	2: # loop end
	
	li a3, 0
	li a4, 0
	2: # loop start
	beq a0, a3, 2f
	addi a3, a3, 1
	addi a4, a4, 4
	j 2b
	2: # loop end
	
	li a3, CLINT_BASE
	add a3, a3, a4
	sw zero, (a3)
	
	1: auipc a3, %pcrel_hi(hart_contexts_user)
	LX a3, %pcrel_lo(1b)(a3)
	add a3, a3, a5
	csrw mscratch, a3
	LX tp, (CT_R_ST + (XLEN_BYTES * REG_TP))(a1)
	beq s0, zero, 2f
	SX tp, (CT_R_ST + (XLEN_BYTES * REG_TP))(s0)
	2:
	lui a2, %tprel_hi(mhartid)
	add a2, a2, tp, %tprel_add(mhartid)
	SX a0, %tprel_lo(mhartid)(a2)
	
	1: auipc a0, %pcrel_hi(interrupt_entry_handler)
	addi a0, a0, %pcrel_lo(1b)
	csrw mtvec, a0

	// Probe for CSRs
	li a0, CSR_MENVCFG
	call probe_csr
	lui a1, %tprel_hi(hart_has_menvcfg)
	add a1, a1, tp, %tprel_add(hart_has_menvcfg)
	SX a0, %tprel_lo(hart_has_menvcfg)(a1)

	# Restore registers only if mscratch was already setup
	beq s0, zero, 2f
	LOAD_REGS REG_S0
	2:
	
	sfence.vma
	fence.i
	mret
	
3: j idle_loop

interrupt_entry_handler:
	sfence.vma
	fence.i
	# Save Register States
	csrrw s0, mscratch, s0 # Save the context pointer in s0
	SAVE_REGS REG_S0
	mv a0, s0
	
	# Save the Program Counter
	csrr a1, mepc
	SX a1, (CT_R_ST + (XLEN_BYTES * REG_PC))(a0)
	
	# Save the Execution Mode
	csrr a1, mstatus
	srli a1, a1, 11
	andi a1, a1, 0x3
	SX a1, (XLEN_BYTES * 1)(a0)
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	# Find the cause of the interrupt
	csrr a1, mcause
	slti ra, a1, 0 # Save the interrupt flag
	not sp, zero
	srli sp, sp, 1
	and a1, a1, sp # Save the cause value in a1 (arg2)
	
	csrr t0, mhartid
	
	1: auipc t1, %pcrel_hi(hart_contexts)
	LX t1, %pcrel_lo(1b)(t1)
	
	1: auipc t2, %pcrel_hi(hart_context_count)
	LX t2, %pcrel_lo(1b)(t2)
	
	li a3, 0
	li a5, 0
	2: # loop start
	beq t2, a3, 3b # Error: mhartid not found in array
	LX a4, (t1)
	beq t0, a4, 2f
	addi t1, t1, CT_SIZE
	addi a5, a5, CT_SIZE
	addi a3, a3, 1
	j 2b
	2: # loop end
	
	LX sp, (CT_R_ST + (XLEN_BYTES * REG_SP))(t1)
	LX tp, (CT_R_ST + (XLEN_BYTES * REG_TP))(t1)
	
	csrr a2, mtval
	
	bne ra, zero, 2f # Jump if interrupt
	
	addi sp, sp, -0x800
	1: auipc a3, %pcrel_hi(hart_contexts_exception)
	LX a3, %pcrel_lo(1b)(a3)
	add a3, a3, a5
	csrw mscratch, a3
	LX a3, 0x08(a0)
	li a4, 3
	beq a3, a4, 4f
	csrsi mstatus, 0x8
	4:
	
	call exception_c_handler
	j 3f
	
	2: call interrupt_c_handler
	
	3: mv a0, s0

switch_context:
	# Ensure that interrupts are disabled so that this is atomic
	csrci mstatus, 0x8
	
	LX a4, (XLEN_BYTES * 1)(a0)   # Load the execution mode from the context block
	LX a1, (CT_R_ST + (XLEN_BYTES * REG_PC))(a0)   # Load the pc address from the context block
	
	# Align the execution mode to the bit field MPP of the mstatus csr
	# MPP: Previous Execution Mode
	andi a4, a4, 0x3
	slli a4, a4, 11
	
	csrr a2, mstatus  # Get mstatus state
	
	# Clear the MPP bits of mstatus
	# MPP: Previous Execution Mode
	li a3, 0x3
	slli a3, a3, 11
	not a3, a3
	and a2, a2, a3
	
	# Set the MPIE bit flag of mstatus
	# MPIE: Previous Interrupt Enable Flag
	ori a2, a2, 0x80
	
	# Write the desired execution mode to the MPP bits of mstatus
	or a2, a2, a4
	
	# Setup the CSRs
	csrw mscratch, a0 # Save the context pointer to mscratch
	csrw mepc, a1     # Set the execution address to switch to
	csrw mstatus, a2  # Set the mstatus state
	
	# Set Register States
	LOAD_REGS REG_A0
	
	# Finally: Jump and switch execution modes
	sfence.vma
	fence.i
	mret
	
	# Should be unreachable.  Jump to an infinite loop just in case.
	j idle_loop

s_delegation_trampoline:
	# Ensure that interrupts are disabled so that this is atomic
	csrci mstatus, 0x8
	
	# Set mscratch in case it was clobbered by an interrupt handler
	csrw mscratch, a0
	
	# Set scause
	# The mcause CSR can not be used because it might have been clobbered by an interrupt handler
	csrw scause, a1
	
	# Set stval
	# The mtval CSR can not be used because it might have been clobbered by an interrupt handler
	csrw stval, a2
	
	# Set the return address
	LX t0, (CT_R_ST + (XLEN_BYTES * REG_PC))(a0)
	csrw sepc, t0
	
	# Find the kernel trap handler
	csrr t0, stvec
	li ra, 1
	andi sp, t0, 0x3
	bne ra, sp, 1f # Jump if not vectored Mode
	bge a1, zero, 1f # Jump if cause is an exception
	#li t2, -1
	#srli ra, t2, 1
	#and t1, a1, ra
	#slli t1, t1, 2
	slli t1, a1, 2
	add t0, t0, t1
	1:
	#slli ra, t2, 2
	#and t0, t0, ra
	andi t0, t0, -4 # May not be needed if the lower 2 bits of mepc are hardwired to zero
	csrw mepc, t0
	
	# Get the current state of mstatus
	csrr t0, mstatus
	
	# Mask of bits to set in mstatus
	andi t0, t0, 0x2          # Clear all bits except for mstatus.SIE
	ori t0, t0, 0x80          # Binary OR in the setting of mstatus.MPP = 1 (Supervisor Mode)
	slli t0, t0, 4 # 0x0820   # Shift it into the correct place
	                          # mstatus.MPP[12:11] = 1 (Supervisor Mode), mstatus.SPIE[5] = mstatus.SIE[1]
	
	# Fake the mstatus.SPP setting such that the Supervisor thinks it received the trap
	LX t1, (XLEN_BYTES * 1)(a0) # Get execution_mode from the cpu_context struct
	andi t1, t1, 0x1            # Truncate the execution_mode to fit into mstatus.SPP
	slli t1, t1, 8              # Shift it into the correct place
	or t0, t0, t1               # Binary OR it against the other bits to set
	
	# Mask of bits to clear in mstatus
	addi t1, zero, 0x19        # Mask of mstatus.MPP, mstatus.SPP
	slli t1, t1, 8 # 0x1900    # Shift it into the correct place
	ori t1, t1, 0x22 # 0x1922  # Mask of mstatus.MPP, mstatus.SPP, mstatus.SPIE, mstatus.SIE
	
	csrc mstatus, t1        # Clear: mstatus.MPP, mstatus.SPP, mstatus.SPIE, mstatus.SIE
	csrs mstatus, t0        # Set: mstatus.MPP = 1 (Supervisor Mode), mstatus.SPP, mstatus.SPIE
	
	# Set Register States
	LOAD_REGS REG_A0
	
	# Finally: Jump and switch execution modes
	sfence.vma zero, zero
	fence.i
	mret
	
	# Should be unreachable.  Jump to an infinite loop just in case.
	j idle_loop


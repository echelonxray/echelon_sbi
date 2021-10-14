.section .text

.globl interrupt_entry_handler
.globl hart_start_entry_handler
.globl switch_context

.align 2, 0
hart_start_entry_handler:
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	csrr a0, mhartid
	
	1: auipc a1, %pcrel_hi(hart_contexts)
	lw a1, %pcrel_lo(1b)(a1)
	
	1: auipc a2, %pcrel_hi(hart_context_count)
	lw a2, %pcrel_lo(1b)(a2)
	
	li a3, 0
	2: # loop start
	beq a2, a3, 3f # Error: mhartid not found in array
	lw a4, 0x00(a1)
	beq a0, a4, 2f
	addi a1, a1, 0x8C
	addi a3, a3, 1
	j 2b
	2: # loop end
	
	lw sp, 0x14(a1)
	lw tp, 0x1C(a1)
	csrw mscratch, a1
	mv a0, a3
	
	# Find the cause of the interrupt
	csrr a2, mcause
	slti a1, a2, 0 # Save the interrupt flag in a1 (arg2)
	not a4, zero
	srli a4, a4, 1
	and a2, a2, a4 # Save the cause value in a2 (arg3)
	
	#li a3, 1
	#slli a4, a3, 8
	#csrs medeleg, a4
	
	call hart_start_c_handler
	#li a1, 3
	#not a1, a1
	#and a0, a0, a1
	csrw mtvec, a0
	mret
	
	3:
	j idle_loop

interrupt_entry_handler:
	# Save Register States
	csrrw a0, mscratch, a0 # Save the context pointer in a0 (arg1)
	sw  ra, 0x010(a0) # Save  x1
	sw  sp, 0x014(a0) # Save  x2
	sw  gp, 0x018(a0) # Save  x3
	sw  tp, 0x01C(a0) # Save  x4
	sw  t0, 0x020(a0) # Save  x5
	sw  t1, 0x024(a0) # Save  x6
	sw  t2, 0x028(a0) # Save  x7
	sw  s0, 0x02C(a0) # Save  x8
	sw  s1, 0x030(a0) # Save  x9
	# Placeholder for " Save x10 " -- Actually saved below
	sw  a1, 0x038(a0) # Save x11
	sw  a2, 0x03C(a0) # Save x12
	sw  a3, 0x040(a0) # Save x13
	sw  a4, 0x044(a0) # Save x14
	sw  a5, 0x048(a0) # Save x15
	sw  a6, 0x04C(a0) # Save x16
	sw  a7, 0x050(a0) # Save x17
	sw  s2, 0x054(a0) # Save x18
	sw  s3, 0x058(a0) # Save x19
	sw  s4, 0x05C(a0) # Save x20
	sw  s5, 0x060(a0) # Save x21
	sw  s6, 0x064(a0) # Save x22
	sw  s7, 0x068(a0) # Save x23
	sw  s8, 0x06C(a0) # Save x24
	sw  s9, 0x070(a0) # Save x25
	sw s10, 0x074(a0) # Save x26
	sw s11, 0x078(a0) # Save x27
	sw  t3, 0x07C(a0) # Save x28
	sw  t4, 0x080(a0) # Save x29
	sw  t5, 0x084(a0) # Save x30
	sw  t6, 0x088(a0) # Save x31
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	# Save x10 -- Actually saved here
	csrrw a1, mscratch, a0
	sw  a1, 0x034(a0)
	
	csrr a1, mstatus
	srli a1, a1, 11
	andi a1, a1, 0x3
	sw a1, 0x004(a0)
	
	# Save the Program Counter
	csrr a2, mepc
	sw  a2, 0x00C(a0)
	
	# Find the cause of the interrupt
	csrr a3, mcause
	slti a2, a3, 0 # Save the interrupt flag in a1 (arg3)
	not sp, zero
	srli sp, sp, 1
	and a3, a3, sp # Save the cause value in a2 (arg4)
	
	csrr t0, mhartid
	
	1: auipc t1, %pcrel_hi(hart_contexts)
	lw t1, %pcrel_lo(1b)(t1)
	
	1: auipc t2, %pcrel_hi(hart_context_count)
	lw t2, %pcrel_lo(1b)(t2)
	
	li a1, 0
	2: # loop start
	beq t2, a1, 3b # Error: mhartid not found in array
	lw sp, 0x00(t1)
	beq t0, sp, 2f
	addi t1, t1, 0x8C
	addi a1, a1, 1
	j 2b
	2: # loop end
	
	lw sp, 0x14(t1)
	lw tp, 0x1C(t1)
	
	mv s0, a0
	call interrupt_c_handler
	mv a0, s0

switch_context:
	lw a4, 0x04(a0)   # Load the execution mode from the context block
	lw a1, 0x0C(a0)   # Load the pc address from the context block
	
	# Align the execution mode to setting into MPP of the mstatus
	# MPP: Previous Execution Mode
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
	lw  ra, 0x010(a0) # Set  x1
	lw  sp, 0x014(a0) # Set  x2
	lw  gp, 0x018(a0) # Set  x3
	lw  tp, 0x01C(a0) # Set  x4
	lw  t0, 0x020(a0) # Set  x5
	lw  t1, 0x024(a0) # Set  x6
	lw  t2, 0x028(a0) # Set  x7
	lw  s0, 0x02C(a0) # Set  x8
	lw  s1, 0x030(a0) # Set  x9
	# Placeholder for " Set x10 " -- Actually saved below
	lw  a1, 0x038(a0) # Set x11
	lw  a2, 0x03C(a0) # Set x12
	lw  a3, 0x040(a0) # Set x13
	lw  a4, 0x044(a0) # Set x14
	lw  a5, 0x048(a0) # Set x15
	lw  a6, 0x04C(a0) # Set x16
	lw  a7, 0x050(a0) # Set x17
	lw  s2, 0x054(a0) # Set x18
	lw  s3, 0x058(a0) # Set x19
	lw  s4, 0x05C(a0) # Set x20
	lw  s5, 0x060(a0) # Set x21
	lw  s6, 0x064(a0) # Set x22
	lw  s7, 0x068(a0) # Set x23
	lw  s8, 0x06C(a0) # Set x24
	lw  s9, 0x070(a0) # Set x25
	lw s10, 0x074(a0) # Set x26
	lw s11, 0x078(a0) # Set x27
	lw  t3, 0x07C(a0) # Set x28
	lw  t4, 0x080(a0) # Set x29
	lw  t5, 0x084(a0) # Set x30
	lw  t6, 0x088(a0) # Set x31
	
	# Set x10 Actually set here
	lw  a0, 0x034(a0)
	
	# Finally: Jump and switch execution modes
	mret
	
	# Should be unreachable.  Jump to an infinite loop just in case.
	j idle_loop

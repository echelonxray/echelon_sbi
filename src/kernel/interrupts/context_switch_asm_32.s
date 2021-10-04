.section .text

.globl interrupt_entry_handler
.globl switch_context

.align 8, 0
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
	
	# Save x10 -- Actually saved here
	csrrw a1, mscratch, a0
	sw  a1, 0x034(a0)
	
	# Save the Program Counter
	csrr a2, mepc
	sw  a2, 0x00C(a0)
	
	# Find the cause of the interrupt
	csrr a2, mcause
	csrr a2, mcause
	slti a1, a2, 0 # Save the interrupt flag in a1 (arg2)
	not sp, zero
	srli sp, sp, 1
	and a2, a2, sp # Save the cause value in a2 (arg3)
	
	# Load the location of symbol KISTACK_TOP into the Stack Pointer
	# This is done using pc relative addressing so that it works
	# across 32-bit, 64-bit, and 128-bit sizes and locations.
	# The placement symbol KISTACK_TOP is determined at build time
	# by the linker script to provide 0x1000 bytes of stack space.
	# Specifically, KISTACK_TOP is set (0x1000 - 0x10) so that its
	# initial value can be used to store up to a 128 bit value.
	# It is aligned to a 16 (0x10) byte boundary.
	stack_top_pc_rel_0:
	auipc sp, %pcrel_hi(KISTACK_TOP)
	addi sp, sp, %pcrel_lo(stack_top_pc_rel_0)
	
	call interrupt_c_handler

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

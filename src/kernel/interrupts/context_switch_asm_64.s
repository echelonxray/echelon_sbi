.section .text

.globl interrupt_entry_handler
.globl hart_start_entry_handler
.globl switch_context

.align 8, 0

hart_start_entry_handler:
	auipc a1, %pcrel_hi(hart_m_contexts)
	ld a1, %pcrel_lo(hart_start_entry_handler)(a1)
	csrr a0, mhartid
	
	j idle_loop # Should be unreachable

interrupt_entry_handler:
	# Save Register States
	csrrw a0, mscratch, a0 # Save the context pointer in a0 (arg1)
	sd  ra, 0x020(a0) # Save  x1
	sd  sp, 0x028(a0) # Save  x2
	sd  gp, 0x030(a0) # Save  x3
	sd  tp, 0x038(a0) # Save  x4
	sd  t0, 0x040(a0) # Save  x5
	sd  t1, 0x048(a0) # Save  x6
	sd  t2, 0x050(a0) # Save  x7
	sd  s0, 0x058(a0) # Save  x8
	sd  s1, 0x060(a0) # Save  x9
	# Placeholder for " Save x10 " -- Actually saved below
	sd  a1, 0x070(a0) # Save x11
	sd  a2, 0x078(a0) # Save x12
	sd  a3, 0x080(a0) # Save x13
	sd  a4, 0x088(a0) # Save x14
	sd  a5, 0x090(a0) # Save x15
	sd  a6, 0x098(a0) # Save x16
	sd  a7, 0x0A0(a0) # Save x17
	sd  s2, 0x0A8(a0) # Save x18
	sd  s3, 0x0B0(a0) # Save x19
	sd  s4, 0x0B8(a0) # Save x20
	sd  s5, 0x0C0(a0) # Save x21
	sd  s6, 0x0C8(a0) # Save x22
	sd  s7, 0x0D0(a0) # Save x23
	sd  s8, 0x0D8(a0) # Save x24
	sd  s9, 0x0E0(a0) # Save x25
	sd s10, 0x0E8(a0) # Save x26
	sd s11, 0x0F0(a0) # Save x27
	sd  t3, 0x0F8(a0) # Save x28
	sd  t4, 0x100(a0) # Save x29
	sd  t5, 0x108(a0) # Save x30
	sd  t6, 0x110(a0) # Save x31
	
	# Save x10 -- Actually saved here
	csrrw a1, mscratch, a0
	sd  a1, 0x068(a0)
	
	# Save the Program Counter
	csrr a2, mepc
	sd  a2, 0x018(a0)
	
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
	ld a4, 0x08(a0)   # Load the execution mode from the context block
	ld a1, 0x18(a0)   # Load the pc address from the context block
	
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
	ld  ra, 0x020(a0) # Set  x1
	ld  sp, 0x028(a0) # Set  x2
	ld  gp, 0x030(a0) # Set  x3
	ld  tp, 0x038(a0) # Set  x4
	ld  t0, 0x040(a0) # Set  x5
	ld  t1, 0x048(a0) # Set  x6
	ld  t2, 0x050(a0) # Set  x7
	ld  s0, 0x058(a0) # Set  x8
	ld  s1, 0x060(a0) # Set  x9
	# Placeholder for " Set x10 " -- Actually saved below
	ld  a1, 0x070(a0) # Set x11
	ld  a2, 0x078(a0) # Set x12
	ld  a3, 0x080(a0) # Set x13
	ld  a4, 0x088(a0) # Set x14
	ld  a5, 0x090(a0) # Set x15
	ld  a6, 0x098(a0) # Set x16
	ld  a7, 0x0A0(a0) # Set x17
	ld  s2, 0x0A8(a0) # Set x18
	ld  s3, 0x0B0(a0) # Set x19
	ld  s4, 0x0B8(a0) # Set x20
	ld  s5, 0x0C0(a0) # Set x21
	ld  s6, 0x0C8(a0) # Set x22
	ld  s7, 0x0D0(a0) # Set x23
	ld  s8, 0x0D8(a0) # Set x24
	ld  s9, 0x0E0(a0) # Set x25
	ld s10, 0x0E8(a0) # Set x26
	ld s11, 0x0F0(a0) # Set x27
	ld  t3, 0x0F8(a0) # Set x28
	ld  t4, 0x100(a0) # Set x29
	ld  t5, 0x108(a0) # Set x30
	ld  t6, 0x110(a0) # Set x31
	
	# Set x10 Actually set here
	ld  a0, 0x068(a0)
	
	# Finally: Jump and switch execution modes
	mret

# Should be unreachable.  Jump to an infinite loop just in case.
j idle_loop

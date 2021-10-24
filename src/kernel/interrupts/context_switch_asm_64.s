.section .text

.globl interrupt_entry_handler
.globl hart_start_entry_handler
.globl switch_context
.globl s_delegation_trampoline

.align 2, 0
hart_start_entry_handler:
	sfence.vma
	fence.i
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	#li a0, 0x80
	#csrs mie, a0
	
	csrr a0, mhartid
	
	1: auipc a1, %pcrel_hi(hart_contexts)
	ld a1, %pcrel_lo(1b)(a1)
	
	1: auipc a2, %pcrel_hi(hart_context_count)
	ld a2, %pcrel_lo(1b)(a2)
	
	li a3, 0
	2: # loop start
	beq a2, a3, 3f # Error: mhartid not found in array
	ld a4, 0x00(a1)
	beq a0, a4, 2f
	addi a1, a1, 0x118
	addi a3, a3, 1
	j 2b
	2: # loop end
	
	ld sp, 0x28(a1)
	ld tp, 0x38(a1)
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
	sfence.vma
	fence.i
	mret
	
	3:
	j idle_loop

interrupt_entry_handler:
	sfence.vma
	fence.i
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
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	# Save x10 -- Actually saved here
	csrrw a1, mscratch, a0
	sd  a1, 0x068(a0)
	
	csrr a1, mstatus
	srli a1, a1, 11
	andi a1, a1, 0x3
	sd a1, 0x008(a0)
	
	# Save the Program Counter
	csrr a2, mepc
	sd  a2, 0x018(a0)
	
	# Find the cause of the interrupt
	csrr a3, mcause
	slti a2, a3, 0 # Save the interrupt flag in a1 (arg3)
	not sp, zero
	srli sp, sp, 1
	and a3, a3, sp # Save the cause value in a2 (arg4)
	
	csrr t0, mhartid
	
	1: auipc t1, %pcrel_hi(hart_contexts)
	ld t1, %pcrel_lo(1b)(t1)
	
	1: auipc t2, %pcrel_hi(hart_context_count)
	ld t2, %pcrel_lo(1b)(t2)
	
	li a1, 0
	2: # loop start
	beq t2, a1, 3b # Error: mhartid not found in array
	ld sp, 0x00(t1)
	beq t0, sp, 2f
	addi t1, t1, 0x118
	addi a1, a1, 1
	j 2b
	2: # loop end
	
	ld sp, 0x28(t1)
	ld tp, 0x38(t1)
	
	mv s0, a0
	call interrupt_c_handler
	mv a0, s0

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
	sfence.vma
	fence.i
	mret
	
	# Should be unreachable.  Jump to an infinite loop just in case.
	j idle_loop

s_delegation_trampoline:
	csrr t0, mcause
	csrw scause, t0
	
	ld t0, 0x18(a0)
	add a1, a1, t0
	csrw sepc, a1
	
	csrr t0, stvec
	csrw mepc, t0
	
	csrr t0, mstatus
	srli t0, t0, 3
	andi t0, t0, 0x100
	li a1, 0x19
	li t1, 0x100
	slli a1, a1, 8
	slli t1, t1, 3
	or t0, t0, t1
	csrc mstatus, a1
	csrs mstatus, t0

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
	sfence.vma
	fence.i
	mret
	
	# Should be unreachable.  Jump to an infinite loop just in case.
	j idle_loop

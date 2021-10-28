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
	sd  ra, 0x020(s0) # Save  x1
	sd  sp, 0x028(s0) # Save  x2
	sd  gp, 0x030(s0) # Save  x3
	sd  tp, 0x038(s0) # Save  x4
	sd  t0, 0x040(s0) # Save  x5
	sd  t1, 0x048(s0) # Save  x6
	sd  t2, 0x050(s0) # Save  x7
	# Placeholder for " Save  x8 " -- Actually saved below
	sd  s1, 0x060(s0) # Save  x9
	sd  a0, 0x068(s0) # Save x10
	sd  a1, 0x070(s0) # Save x11
	sd  a2, 0x078(s0) # Save x12
	sd  a3, 0x080(s0) # Save x13
	sd  a4, 0x088(s0) # Save x14
	sd  a5, 0x090(s0) # Save x15
	sd  a6, 0x098(s0) # Save x16
	sd  a7, 0x0A0(s0) # Save x17
	sd  s2, 0x0A8(s0) # Save x18
	sd  s3, 0x0B0(s0) # Save x19
	sd  s4, 0x0B8(s0) # Save x20
	sd  s5, 0x0C0(s0) # Save x21
	sd  s6, 0x0C8(s0) # Save x22
	sd  s7, 0x0D0(s0) # Save x23
	sd  s8, 0x0D8(s0) # Save x24
	sd  s9, 0x0E0(s0) # Save x25
	sd s10, 0x0E8(s0) # Save x26
	sd s11, 0x0F0(s0) # Save x27
	sd  t3, 0x0F8(s0) # Save x28
	sd  t4, 0x100(s0) # Save x29
	sd  t5, 0x108(s0) # Save x30
	sd  t6, 0x110(s0) # Save x31
	
	# Save x8 -- Actually saved here
	csrrw a0, mscratch, s0
	sd  a0, 0x058(s0)
	2:
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	csrr a0, mhartid
	
	1: auipc a1, %pcrel_hi(hart_contexts)
	ld a1, %pcrel_lo(1b)(a1)
	
	1: auipc a2, %pcrel_hi(hart_context_count)
	ld a2, %pcrel_lo(1b)(a2)
	
	li a3, 0
	li a5, 0
	2: # loop start
	beq a2, a3, 3f # Error: mhartid not found in array
	ld a4, 0x00(a1)
	beq a0, a4, 2f
	addi a1, a1, 0x118
	addi a5, a5, 0x118
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
	#li a3, 0x02000000
	add a3, a3, a4
	sw zero, (a3)
	
	1: auipc a3, %pcrel_hi(hart_contexts_exception)
	ld a3, %pcrel_lo(1b)(a3)
	add a3, a3, a5
	csrw mscratch, a3
	ld tp, 0x38(a1)
	lui a2, %tprel_hi(mhartid)
	add a2, a2, tp, %tprel_add(mhartid)
	sd a0, %tprel_lo(mhartid)(a2)
	
	1: auipc a0, %pcrel_hi(interrupt_entry_handler)
	addi a0, a0, %pcrel_lo(1b)
	csrw mtvec, a0
	
	# Save registers only if mscratch was already setup
	beq s0, zero, 2f
	ld  ra, 0x020(s0) # Set  x1
	ld  sp, 0x028(s0) # Set  x2
	ld  gp, 0x030(s0) # Set  x3
	ld  tp, 0x038(s0) # Set  x4
	ld  t0, 0x040(s0) # Set  x5
	ld  t1, 0x048(s0) # Set  x6
	ld  t2, 0x050(s0) # Set  x7
	# Placeholder for " Set  x8 " -- Actually set below
	ld  s1, 0x060(s0) # Set  x9
	ld  a0, 0x068(s0) # Set x10
	ld  a1, 0x070(s0) # Set x11
	ld  a2, 0x078(s0) # Set x12
	ld  a3, 0x080(s0) # Set x13
	ld  a4, 0x088(s0) # Set x14
	ld  a5, 0x090(s0) # Set x15
	ld  a6, 0x098(s0) # Set x16
	ld  a7, 0x0A0(s0) # Set x17
	ld  s2, 0x0A8(s0) # Set x18
	ld  s3, 0x0B0(s0) # Set x19
	ld  s4, 0x0B8(s0) # Set x20
	ld  s5, 0x0C0(s0) # Set x21
	ld  s6, 0x0C8(s0) # Set x22
	ld  s7, 0x0D0(s0) # Set x23
	ld  s8, 0x0D8(s0) # Set x24
	ld  s9, 0x0E0(s0) # Set x25
	ld s10, 0x0E8(s0) # Set x26
	ld s11, 0x0F0(s0) # Set x27
	ld  t3, 0x0F8(s0) # Set x28
	ld  t4, 0x100(s0) # Set x29
	ld  t5, 0x108(s0) # Set x30
	ld  t6, 0x110(s0) # Set x31
	
	# Set x8 -- Actually set here
	ld  s0, 0x058(s0)
	2:
	
	sfence.vma
	fence.i
	mret
	
3: j idle_loop

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
	
	# Save x10 -- Actually saved here
	csrrw a1, mscratch, a0
	sd  a1, 0x068(a0)
	
	# Save the Program Counter
	csrr a1, mepc
	sd  a1, 0x018(a0)
	
	# Save the Execution Mode
	csrr a1, mstatus
	srli a1, a1, 11
	andi a1, a1, 0x3
	sd a1, 0x008(a0)
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	# Find the cause of the interrupt
	csrr a1, mcause
	slti a2, a1, 0 # Save the interrupt flag in a2
	not sp, zero
	srli sp, sp, 1
	and a1, a1, sp # Save the cause value in a1 (arg2)
	
	csrr t0, mhartid
	
	1: auipc t1, %pcrel_hi(hart_contexts)
	ld t1, %pcrel_lo(1b)(t1)
	
	1: auipc t2, %pcrel_hi(hart_context_count)
	ld t2, %pcrel_lo(1b)(t2)
	
	li a3, 0
	li a5, 0
	2: # loop start
	beq t2, a3, 3b # Error: mhartid not found in array
	ld a4, 0x00(t1)
	beq t0, a4, 2f
	addi t1, t1, 0x118
	addi a5, a5, 0x118
	addi a3, a3, 1
	j 2b
	2: # loop end
	
	ld sp, 0x28(t1)
	ld tp, 0x38(t1)
	
	mv s0, a0
	
	bne a2, zero, 2f # Jump if interrupt
	addi sp, sp, -0x800
	1: auipc a3, %pcrel_hi(hart_contexts_exception)
	addi a3, a3, %pcrel_lo(1b)
	add a3, a3, a5
	csrw mscratch, a3
	csrsi mstatus, 0x8
	call exception_c_handler
	j 3f
	
	2: call interrupt_c_handler
	
	3: mv a0, s0

switch_context:
	csrci mstatus, 0x8
	
	ld a4, 0x08(a0)   # Load the execution mode from the context block
	ld a1, 0x18(a0)   # Load the pc address from the context block
	
	# Align the execution mode to the bit field MPP of the mstatus csr
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
	# Placeholder for " Set x10 " -- Actually set below
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
	sfence.vma zero, zero
	fence.i
	mret
	
	# Should be unreachable.  Jump to an infinite loop just in case.
	j idle_loop

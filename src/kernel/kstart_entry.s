.section entry_text

.globl my_entry_pt
.globl idle_loop

my_entry_pt:
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	mv t0, a0
	
	1: auipc a0, %pcrel_hi(hart_init_handler)
	addi a0, a0, %pcrel_lo(1b)
	csrw mtvec, a0
	
	li a0, 0x08
	csrw mscratch, zero
	csrw mie, a0
	csrs mstatus, a0
	
	# Spin/Halt all harts except for hart specified
	li t1, 0
	csrr a0, mhartid
	bne a0, t1, idle_loop
	
	1: auipc t1, %pcrel_hi(init_reg_a0)
	sd t0, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a1)
	sd a1, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a2)
	sd a2, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a3)
	sd a3, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a4)
	sd a4, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a5)
	sd a5, %pcrel_lo(1b)(t1)
	
	# Initialize global variables if needed
	1: auipc t0, %pcrel_hi(INIT_DATA_PROGAMIMAGE_START)
	addi t0, t0, %pcrel_lo(1b)
	1: auipc a1, %pcrel_hi(INIT_DATA_RUNTIME_START)
	addi a1, a1, %pcrel_lo(1b)
	# If the addresses are different, the global variables
	# are separate from the program binary.  Copy over
	# the image of to initialized values to memory in order
	# to initialize the global variables in RAM.
	beq t0, a1, 2f
	1: auipc a2, %pcrel_hi(INIT_DATA_RUNTIME_END)
	addi a2, a2, %pcrel_lo(1b)
	3: beq a1, a2, 2f
	lb a3, (t0)
	sb a3, (a1)
	addi t0, t0, 1
	addi a1, a1, 1
	j 3b
	2:
	
	# Load the location of symbol KISTACK_TOP into the Stack Pointer
	# This is done using pc relative addressing so that it works
	# across 32-bit, 64-bit, and 128-bit sizes and locations.
	# The placement symbol KISTACK_TOP is determined at build time
	# by the linker script to provide 0x1000 bytes of stack space.
	# Specifically, KISTACK_TOP is set (0x1000 - 0x10) so that its
	# initial value can be used to store up to a 128 bit value.
	# It is aligned to a 16 (0x10) byte boundary.
	1: auipc sp, %pcrel_hi(KISTACK_TOP)
	addi sp, sp, %pcrel_lo(1b)
	
	# Call into the C functions
	call kinit # Register tp is initialized inside this function.
	call kmain

idle_loop:
	wfi
	j idle_loop

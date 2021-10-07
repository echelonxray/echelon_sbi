.section entry_text

.globl my_entry_pt
.globl idle_loop

my_entry_pt:
	csrr a3, mhartid
	
	lui a1, 0x40008
	slli a1, a1, 1
	li a4, 1
	sll a4, a4, a3
	amoor.w zero, a4, (a1)
	
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	bne a3, zero, setup_int_and_spin
	
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
	
	# Call into the C function
	call kmain

idle_loop:
	wfi
	j idle_loop

setup_int_and_spin:
	# Setup the interrupt vector
	1: auipc a0, %pcrel_hi(hart_start_entry_handler)
	addi a0, a0, %pcrel_lo(1b)
	csrw mtvec, a0
	li a0, 0x08
	csrs mie, a0
	csrs mstatus, a0
	j idle_loop

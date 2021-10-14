.section .text

.globl my_entry_pt

my_entry_pt:
	# Spin all harts except for hart 0
	#csrr a0, mhartid
	#bne a0, zero, loop
	
	# Setup the Global Pointer
	.option push
	.option norelax
	global_pointer_pc_rel_0:
	auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(global_pointer_pc_rel_0)
	.option pop
	
	lui a1, 0x10010
	li a0, 1
	sw a0, 0x08(a1)
	
	find_my_string:
	auipc a0, %pcrel_hi(my_string)
	addi a0, a0, %pcrel_lo(find_my_string)
	
	print_string:
	lw a2, (a0)
	beq a2, zero, loop
	sw a2, (a1)
	addi a0, a0, 1
	j print_string
	
	loop:
	wfi
	j loop

.section .rodata
	my_string: .string "--Test Program Running--\n\x00"

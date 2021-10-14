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
	auipc a0, %pcrel_hi(my_string1)
	addi a0, a0, %pcrel_lo(find_my_string)
	
	call print_string
	
	ecall
	
	1: auipc a0, %pcrel_hi(my_string2)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	1: auipc a0, %pcrel_hi(trap_handler)
	addi a0, a0, %pcrel_lo(1b)
	csrw stvec, a0
	
	1: auipc a0, %pcrel_hi(umode_program)
	addi a0, a0, %pcrel_lo(1b)
	csrw sepc, a0
	
	#li a0, 0x20
	#csrs sstatus, a0
	
	sret
	
	loop:
	wfi
	j loop

print_string:
	lbu a2, (a0)
	beq a2, zero, 1f
	sw a2, (a1)
	addi a0, a0, 1
	j print_string
	1: ret
	j loop

trap_handler:
	lui a1, 0x10010
	
	1: auipc a0, %pcrel_hi(my_string3)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	csrr a0, sepc
	addi a0, a0, 4
	csrw sepc, a0
	sret
	j loop

umode_program:
	ecall
	ecall
	ecall
	j loop

.section .rodata
	my_string1: .string "--Test Program Running--\n\x00"
	my_stringa: .string "<TraceA>\n\x00"
	my_string2: .string "Switching to U-Mode\n\x00"
	my_stringb: .string "<TraceB>\n\x00"
	my_string3: .string "ESBI Trap Caught!  Trap Handler: S-Mode\n\x00"
	my_stringc: .string "<TraceC>\n\x00"

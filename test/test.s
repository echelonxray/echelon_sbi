.global my_entry_pt

.section .text
.align 2, 0
my_entry_pt: j main_code
.align 2, 0
.4byte 0
.8byte 0x00200000 # 0x0020_0000
.8byte (program_end - my_entry_pt)
.8byte 0
.2byte 2
.2byte 0
.4byte 0
.8byte 0
.8byte 0x0000056435349520 # 0x0000_0564_3534_9520
.4byte 0x05435352 # 0x0543_5352
.4byte 0

main_code:
	# Spin all harts except for hart 0
	#csrr a0, mhartid
	#bne a0, zero, loop
	
	mv s0, a0
	mv s1, a1
	mv s2, a2
	mv s3, a3
	mv s4, a4
	
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
	
	# -----------------------------------------
	mv a0, s0
	call itoa
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	# -----------------------------------------
	# -----------------------------------------
	mv a0, s1
	call itoa
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	# -----------------------------------------
	# -----------------------------------------
	mv a0, s2
	call itoa
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	# -----------------------------------------
	# -----------------------------------------
	mv a0, s3
	call itoa
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	# -----------------------------------------
	# -----------------------------------------
	mv a0, s4
	call itoa
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	# -----------------------------------------
	
	j loop
	
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
	2: lw a3, (a1)
	blt a3, zero, 2b
	sw a2, (a1)
	addi a0, a0, 1
	j print_string
	1: ret

itoa:
	1: auipc t0, %pcrel_hi(program_end)
	addi t0, t0, %pcrel_lo(1b)
	li t1, 10
	beqz a0, 2f
	1: beqz a0, 1f
	remu t2, a0, t1
	divu a0, a0, t1
	sb t2, (t0)
	addi t0, t0, 1
	j 1b
	2: sb zero, (t0)
	addi t0, t0, 1
	1: sb zero, (t0)
	1: auipc t1, %pcrel_hi(program_end)
	addi t1, t1, %pcrel_lo(1b)
	2: bgeu t1, t0, 1f
	addi t0, t0, -1
	lbu t2, (t0)
	lbu a2, (t1)
	addi t2, t2, '0'
	addi a2, a2, '0'
	sb t2, (t1)
	sb a2, (t0)
	addi t1, t1, 1
	j 2b
	1: ret

j loop # Should be unreachable

trap_handler:
	lui a1, 0x10010
	
	1: auipc a0, %pcrel_hi(my_string3)
	addi a0, a0, %pcrel_lo(1b)
	
	call print_string
	
	ecall
	
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
	my_string4: .string "\n\x00"
	my_stringd: .string "<TraceD>\n\x00"

program_end:

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
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	lui a1, 0x10010
	li a0, 1
	sw a0, 0x08(a1)
	
	find_my_string:
	auipc a0, %pcrel_hi(my_string1)
	addi a0, a0, %pcrel_lo(find_my_string)
	call print_string
	
	li a7, 0x10
	li a6, 3
	#li a0, 0x54494D45 # Ext: TIME
	#li a0, 0x735049   # Ext: IPI
	#li a0, 0x52464E43 # Ext: RFNC
	#li a0, 0x48534D   # Ext: HSM
	#li a0, 0x53525354 # Ext: SRST
	#li a0, 0x504D55   # Ext: PMU
	#li a0, 0x11       # Ext: [Does Not Exist]
	li a0, 0x10       # Ext: [Does Not Exist]
	ecall
	
	mv a4, a1
	lui a1, 0x10010
	call itoa
	
	1: auipc a0, %pcrel_hi(my_string2)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string3)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	mv a0, a4
	call itoa
	
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	call print_string

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

.section .rodata
	my_string1: .string "--Test Program Running--\n\x00"
	my_stringa: .string "<TraceA>\n\x00"
	my_string2: .string "Result REG_A0 Error: \x00"
	my_stringb: .string "<TraceB>\n\x00"
	my_string3: .string "Result REG_A1 Value: \x00"
	my_stringc: .string "<TraceC>\n\x00"
	my_string4: .string "\n\x00"
	my_stringd: .string "<TraceD>\n\x00"

program_end:

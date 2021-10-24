.option norelax
.global my_entry_pt

.section .text
.align 0, 0
my_entry_pt:
	j main_code
	add zero, zero, zero
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

trap_handler_offset: .8byte (trap_handler - my_entry_pt)
resume_from_vma_offset: .8byte (resume_from_vma_activation - my_entry_pt)

main_code:
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	lui a1, 0x10010
	li a0, 1
	sw a0, 0x08(a1)
	
	1: auipc a0, %pcrel_hi(my_string1)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	# Setup STVEC CSR
	lui t0, 0xC0000
	1: auipc t1, %pcrel_hi(trap_handler_offset)
	ld t1, %pcrel_lo(1b)(t1)
	add t0, t0, t1
	csrw stvec, t0
	
	# Setup Page Table
	addi t0, zero, -1
	slli t0, t0, 12
	1: auipc a0, %pcrel_hi(program_end)
	addi a0, a0, %pcrel_lo(1b)
	1: auipc t1, %pcrel_hi(my_entry_pt)
	addi t1, t1, %pcrel_lo(1b)
	lui a4, 0x10010
	and a0, a0, t0
	and t1, t1, t0
	lui t2, 0x00001
	add a0, a0, t2
	add a1, a0, t2
	add a2, a1, t2
	add a3, a2, t2
	
	srai t1, t1, 2
	ori t1, t1, 0x4F
	sd t1, 0x0(a3)
	srai a4, a4, 2
	ori a4, a4, 0x4F
	sd a4, 0x80(a3)
	
	mv t1, a3
	and t1, t1, t0
	srai t1, t1, 2
	ori t1, t1, 0x41
	sd t1, 0x0(a2)
	sd t1, 0x400(a2)
	
	mv t1, a2
	and t1, t1, t0
	srai t1, t1, 2
	ori t1, t1, 0x41
	sd t1, -0x8(a1)
	sd t1, 0x0(a0)
	
	# Setup SATP CSR
	srli a0, a0, 12
	addi t1, zero, -1
	srli t1, t1, 20
	and a0, a0, t1
	addi t1, zero, 0x8
	slli t1, t1, 60
	or a0, a0, t1
	csrw satp, a0
	sfence.vma zero, zero
	fence.i
	add zero, zero, zero
	
	#csrr a0, satp
	#mv a0, s0
	#call itoa
	
  lui a1, 0x10010
  
  #1: auipc a0, %pcrel_hi(program_end)
	#addi a0, a0, %pcrel_lo(1b)
	#call print_string
  #1: auipc a0, %pcrel_hi(my_string6)
	#addi a0, a0, %pcrel_lo(1b)
	#call print_string
	
  1: auipc a0, %pcrel_hi(my_string5)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
resume_from_vma_activation:
  lui a1, 0x10010
  
  1: auipc a0, %pcrel_hi(my_string3)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
  1: auipc a0, %pcrel_hi(my_string4)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
	
	#.4byte 0

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

trap_handler:
  lui a1, 0x10010
  1: auipc a0, %pcrel_hi(my_string2)
	addi a0, a0, %pcrel_lo(1b)
	call print_string
  
	lui t0, 0xC0000
	1: auipc t1, %pcrel_hi(resume_from_vma_offset)
	ld t1, %pcrel_lo(1b)(t1)
	add t0, t0, t1
	csrw sepc, t0
	sret

j loop # Should be unreachable

.section .rodata
	my_string1: .string "Test Program Running\n\x00"
	my_stringa: .string "<TraceA>\n\x00"
	my_string2: .string "Trap Taken\n\x00"
	my_stringb: .string "<TraceB>\n\x00"
	my_string3: .string "Successful Resume From Trap\n\x00"
	my_stringc: .string "<TraceC>\n\x00"
	my_string4: .string "Program Finished: Spinning\n\x00"
	my_stringd: .string "<TraceD>\n\x00"
	my_string5: .string "Error: Unreachable Reached\n\x00"
	my_stringe: .string "<TraceE>\n\x00"
	my_string6: .string "\n\x00"
	my_stringf: .string "<TraceF>\n\x00"

program_end:

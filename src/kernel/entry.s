.section .text

.globl my_entry_pt

my_entry_pt:
  csrr a0, mvendorid
  csrr a1, marchid
  csrr a2, mimpid
  csrr a3, mhartid
  lui a4, 0x40010
  slli a4, a4, 1
  li s1, 32
  mv s2, a3
  call mul
  add a5, s3, a4
  sw a0, 0(a5)
  sw a1, 8(a5)
  sw a2, 16(a5)
  sw a3, 24(a5)
  
  li a2, 1
  bne a3, a2, loop
  
  #csrw satp, t6
  #csrr t6, satp
  #csrr t6, mstatus
  
  # Load 0x8001FFF0 into Stack Pointer
  ori a0, zero, 0x1FF
  slli a0, a0, 4
  lui sp, 0x4000F
  slli sp, sp, 1
  or sp, sp, a0
  
  mv ra, zero
  mv gp, zero
  mv tp, zero
  mv t0, zero
  mv t1, zero
  mv t2, zero
  mv fp, zero
  mv s1, zero
  mv a0, zero
  mv a1, zero
  mv a2, zero
  mv a3, zero
  mv a4, zero
  mv a5, zero
  mv a6, zero
  mv a7, zero
  mv s2, zero
  mv s3, zero
  mv s4, zero
  mv s5, zero
  mv s6, zero
  mv s7, zero
  mv s8, zero
  mv s9, zero
  mv s10, zero
  mv s11, zero
  mv t3, zero
  mv t4, zero
  mv t5, zero
  mv t6, zero
  
  call kmain

loop:
  wfi
  j loop

mul:
  mv s3, zero
  mv t0, zero
  mul_loop:
  beq t0, s2, mul_end
  addi t0, t0, 1
  add s3, s3, s1
  j mul_loop
  mul_end:
  ret

j loop

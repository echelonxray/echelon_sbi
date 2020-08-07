.section .text_test

.globl my_entry_pt

my_entry_pt:
  mv ra, zero
  mv sp, zero
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
  
  # Load 0x80003FFF into Stack Pointer and Global Pointer
  ori a0, zero, 0xF
  slli a0, a0, 8
  ori a0, a0, 0xFF
  lui sp, 0x80003
  or sp, sp, a0
  mv a0, zero
  # not sp, zero
  mv sp, gp
  
  call main
  
  loop:
  nop
  nop
  nop
  nop
  nop
  j loop

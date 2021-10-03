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
  #sw a0, 0(a5)
  #sw a1, 8(a5)
  #sw a2, 16(a5)
  #sw a3, 24(a5)
  
  li a2, 1
  bne a3, a2, clear_and_loop
  
  # Load the location of symbol KISTACK_TOP into Stack Pointer
  # This is done using pc relative addressing so that it works
  # across 32-bit, 64-bit, and 128-bit sizes and locations.
  stack_top_pc_rel_0:
  auipc sp, %pcrel_hi(KISTACK_TOP)
  addi sp, sp, %pcrel_lo(stack_top_pc_rel_0)
  
  # Zero all other registers
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
  
  # Call into the C function
  call kmain

clear_and_loop:
  mv sp, zero
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
  loop:
  wfi
  j loop

# Primative multiplication of registers s1 with s2. It
# stores the result in s3 and returns.  It assumes
# positive numbers in both s1 and s2.  It also does not
# handle overflow.  It does not change the values of s1
# or s2.  In addition to s3, it clobbers t0.
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

# This should not be reachable, but just in case,
# jump back into the infinite loop.
j clear_and_loop

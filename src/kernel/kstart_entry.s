.section .text

.globl my_entry_pt
.globl idle_loop

my_entry_pt:
  csrr a3, mhartid
  
  li a2, 1
  bne a3, a2, clear_and_loop
  
  # Load the location of symbol KISTACK_TOP into the Stack Pointer
  # This is done using pc relative addressing so that it works
  # across 32-bit, 64-bit, and 128-bit sizes and locations.
  # The placement symbol KISTACK_TOP is determined at build time
  # by the linker script to provide 0x1000 bytes of stack space.
  # Specifically, KISTACK_TOP is set (0x1000 - 0x10) so that its
  # initial value can be used to store up to a 128 bit value.
  # It is aligned to a 16 (0x10) byte boundary.
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
idle_loop:
  wfi
  j idle_loop

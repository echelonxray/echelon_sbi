.section .text

.globl interrupt_handler

.align 8, 0
interrupt_handler:
  not sp, sp
  ori sp, sp, 0xF
  not sp, sp
  
  sw ra, -0x04(sp)
  sw sp, -0x08(sp)
  sw gp, -0x0C(sp)
  sw tp, -0x10(sp)
  sw t0, -0x14(sp)
  sw t1, -0x18(sp)
  sw t2, -0x1C(sp)
  sw s0, -0x20(sp)
  sw s1, -0x24(sp)
  sw a0, -0x28(sp)
  sw a1, -0x2C(sp)
  sw a2, -0x30(sp)
  sw a3, -0x34(sp)
  sw a4, -0x38(sp)
  sw a5, -0x3C(sp)
  sw a6, -0x40(sp)
  sw a7, -0x44(sp)
  sw s2, -0x48(sp)
  sw s3, -0x4C(sp)
  sw s4, -0x50(sp)
  sw s5, -0x54(sp)
  sw s6, -0x58(sp)
  sw s7, -0x5C(sp)
  sw s8, -0x60(sp)
  sw s9, -0x64(sp)
  sw s10, -0x68(sp)
  sw s11, -0x6C(sp)
  sw t3, -0x70(sp)
  sw t4, -0x74(sp)
  sw t5, -0x78(sp)
  sw t6, -0x7C(sp)
  addi sp, sp, -0x80
  
  call interrupt_chandle
  
  addi sp, sp, 0x80
  lw ra, -0x04(sp)
  lw sp, -0x08(sp)
  lw gp, -0x0C(sp)
  lw tp, -0x10(sp)
  lw t0, -0x14(sp)
  lw t1, -0x18(sp)
  lw t2, -0x1C(sp)
  lw s0, -0x20(sp)
  lw s1, -0x24(sp)
  lw a0, -0x28(sp)
  lw a1, -0x2C(sp)
  lw a2, -0x30(sp)
  lw a3, -0x34(sp)
  lw a4, -0x38(sp)
  lw a5, -0x3C(sp)
  lw a6, -0x40(sp)
  lw a7, -0x44(sp)
  lw s2, -0x48(sp)
  lw s3, -0x4C(sp)
  lw s4, -0x50(sp)
  lw s5, -0x54(sp)
  lw s6, -0x58(sp)
  lw s7, -0x5C(sp)
  lw s8, -0x60(sp)
  lw s9, -0x64(sp)
  lw s10, -0x68(sp)
  lw s11, -0x6C(sp)
  lw t3, -0x70(sp)
  lw t4, -0x74(sp)
  lw t5, -0x78(sp)
  lw t6, -0x7C(sp)
  mret
  
  j idle_loop

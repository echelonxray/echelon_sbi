.section .text

.globl interrupt_handler

.align 8, 0
interrupt_handler:
  csrrw a0, mscratch, a0
  sw ra, 0x10(a0)
  sw sp, 0x14(a0)
  sw gp, 0x18(a0)
  sw tp, 0x1C(a0)
  sw t0, 0x20(a0)
  sw t1, 0x24(a0)
  sw t2, 0x28(a0)
  sw s0, 0x2C(a0)
  sw s1, 0x30(a0)
  sw a1, 0x38(a0)
  sw a2, 0x3C(a0)
  sw a3, 0x40(a0)
  sw a4, 0x44(a0)
  sw a5, 0x48(a0)
  sw a6, 0x4C(a0)
  sw a7, 0x50(a0)
  sw s2, 0x54(a0)
  sw s3, 0x58(a0)
  sw s4, 0x5C(a0)
  sw s5, 0x60(a0)
  sw s6, 0x64(a0)
  sw s7, 0x68(a0)
  sw s8, 0x6C(a0)
  sw s9, 0x70(a0)
  sw s10, 0x74(a0)
  sw s11, 0x78(a0)
  sw t3, 0x7C(a0)
  sw t4, 0x80(a0)
  sw t5, 0x84(a0)
  sw t6, 0x88(a0)
  csrrw a1, mscratch, a0
  sw a1, 0x34(a0)
  
  call interrupt_chandle
  
  csrrc a0, mscratch, zero
  csrrc a1, mepc, zero
  addi a1, a1, 4
  csrrw zero, mepc, a1
  lw ra, 0x10(a0)
  lw sp, 0x14(a0)
  lw gp, 0x18(a0)
  lw tp, 0x1C(a0)
  lw t0, 0x20(a0)
  lw t1, 0x24(a0)
  lw t2, 0x28(a0)
  lw s0, 0x2C(a0)
  lw s1, 0x30(a0)
  lw a1, 0x38(a0)
  lw a2, 0x3C(a0)
  lw a3, 0x40(a0)
  lw a4, 0x44(a0)
  lw a5, 0x48(a0)
  lw a6, 0x4C(a0)
  lw a7, 0x50(a0)
  lw s2, 0x54(a0)
  lw s3, 0x58(a0)
  lw s4, 0x5C(a0)
  lw s5, 0x60(a0)
  lw s6, 0x64(a0)
  lw s7, 0x68(a0)
  lw s8, 0x6C(a0)
  lw s9, 0x70(a0)
  lw s10, 0x74(a0)
  lw s11, 0x78(a0)
  lw t3, 0x7C(a0)
  lw t4, 0x80(a0)
  lw t5, 0x84(a0)
  lw t6, 0x88(a0)
  lw a0, 0x34(a0)
  mret
  
  j idle_loop

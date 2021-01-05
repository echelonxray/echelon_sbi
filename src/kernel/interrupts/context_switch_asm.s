.globl switch_context

.align 8, 0
switch_context:
  lw a1, 0x0C(a0)
  lw s0, 0x04(a0)
  li s2, 0x3
  slli s0, s0, 11
  slli s2, s2, 11
  and s1, s0, s2
  ori s1, s1, 0x80
  csrrc zero, mstatus, s2
  csrrs zero, mstatus, s1
  csrrw zero, mscratch, a0
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

loop:
  j loop

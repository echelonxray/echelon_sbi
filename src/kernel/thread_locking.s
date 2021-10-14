.section .text

.globl ksem_wait
.globl ksem_post
.globl ksem_init

ksem_wait:
	li t0, 1
	1: lw t1, (a0)
	bne t1, zero, 1b
	amoswap.w.aq t1, t0, (a0)
	bne t1, zero, 1b
	ret

ksem_post:
	amoswap.w.rl zero, zero, (a0)
	ret

ksem_init:
	amoswap.w.aq zero, zero, (a0)
	ret

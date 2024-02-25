#include <interrupts/context_switch_asm.h>

.section entry_text,"ax"

.globl my_entry_pt
.globl idle_loop
.globl probe_csr

my_entry_pt:
	# Setup the Global Pointer
	.option push
	.option norelax
	1: auipc gp, %pcrel_hi(__global_pointer$)
	addi gp, gp, %pcrel_lo(1b)
	.option pop
	
	# Point the interrupt vector to handle hardware thread init
	1: auipc t0, %pcrel_hi(hart_init_handler)
	addi t0, t0, %pcrel_lo(1b)
	csrw mtvec, t0
	
	li t0, 0x08
	csrw mscratch, zero
	csrw mie, t0
	csrs mstatus, t0
	
	# Spin/Halt all harts except for hart 0
	li t1, 0
	csrr t0, mhartid
	bne t0, t1, idle_loop
	
	# Save the values of a0 through a5 on reset
	1: auipc t1, %pcrel_hi(init_reg_a0)
	SX a0, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a1)
	SX a1, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a2)
	SX a2, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a3)
	SX a3, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a4)
	SX a4, %pcrel_lo(1b)(t1)
	1: auipc t1, %pcrel_hi(init_reg_a5)
	SX a5, %pcrel_lo(1b)(t1)
	
	mv a0, t0
	
	# Initialize global variables if needed
	1: auipc t1, %pcrel_hi(INIT_DATA_PROGAMIMAGE_START)
	addi t1, t1, %pcrel_lo(1b)
	1: auipc a1, %pcrel_hi(INIT_DATA_RUNTIME_START)
	addi a1, a1, %pcrel_lo(1b)
	# If the addresses are different, the global variables
	# are separate from the program binary (RAM/ROM Split).
	# Copy over the image of initialized values to memory in
	# order to initialize the global variables in RAM.
	beq t1, a1, 2f
	1: auipc a2, %pcrel_hi(INIT_DATA_RUNTIME_END)
	addi a2, a2, %pcrel_lo(1b)
	3: beq a1, a2, 2f
	lb a3, (t1)
	sb a3, (a1)
	addi t1, t1, 1
	addi a1, a1, 1
	j 3b
	2:
	
	# Load the location of symbol KISTACK_TOP into the Stack Pointer.
	# The placement symbol KISTACK_TOP is determined at link-time
	# by the linker script to provide 0x1000 bytes of stack space.
	# Specifically, KISTACK_TOP is set (0x1000 - 0x10) so that its
	# initial value can be used to store up to a 128 bit value.
	# It is aligned to a 16 (0x10) byte boundary.
	1: auipc sp, %pcrel_hi(KISTACK_TOP)
	addi sp, sp, %pcrel_lo(1b)
	
	# Call into the C functions
	call kinit # Register tp is initialized inside this function.
	call kmain

idle_loop:
	wfi
	j idle_loop

// C prototype:
//              unsigned long probe_csr(unsigned long csr_address);
//
// Description:
//              Probe for the existence of a CSR.
//
// Parameters:
//              csr_address refers to the address of the CSR to probe.  Only the lower
//              12 bits of the csr_address parameter are used.  The rest are ignored.
//
// Return values:
//              Returns 1 if it exists.  Returns 0 if it doesn't exist.
//
// Limitations:
//              This function internally uses and therefore can not probe and should never
//              be called to probe CSRs:
//               - mstatus
//               - mtvec
//               - mtval
//               - mepc
//               - mcause
//              This function uses and depends on stack execution.  If stack execution throws
//              an exception, perhaps because of stack execution restrictions, this function
//              will always and erroniously return 0.
//
probe_csr:
	1: auipc t1, %pcrel_hi(3f) // Get address of temporary trap handler
	addi t1, t1, %pcrel_lo(1b)
	1: auipc a4, %pcrel_hi(2f) // Get address of return pathway
	addi a4, a4, %pcrel_lo(1b)
	1: auipc a1, %pcrel_hi(4f) // Get address of template instructions
	addi a1, a1, %pcrel_lo(1b)

	// Save and configure used CSRs
	csrrci t0, mstatus, 0x8 // Disable interrupts and save previous state
	csrrw  t1, mtvec, t1 // Save and override trap vector
	csrr   t2, mtval
	csrr   t3, mepc
	csrr   t4, mcause

	// Copy the constant parts of the executable instructions
	lh a2, 0x0(a1)
	lw a3, 0x4(a1)
	sh a2, -0x8(sp)
	sw a3, -0x4(sp)

	// Load-modify-write the modified portion of the instructions
	lb a2, 0x2(a1)
	slli a3, a0, 4
	andi a2, a2, 0x0F
	srli a0, a0, 4
	or   a2, a2, a3
	sb a0, -0x5(sp)
	sb a2, -0x6(sp)

	// Attempt to access the CSR
	li a0, 1 // Set the return value for success
	jalr zero, -0x8(sp)

	3: // Temporary trap handler
	li a0, 0 // Set the return value for failure.  (Overwrites previously stored success)

	2: // Restore values and return
	csrw mcause,  t4
	csrw mepc,    t3
	csrw mtval,   t2
	csrw mtvec,   t1
	csrw mstatus, t0
	ret

	// Template of instructions used to attempt CSR access.
	// This is:
	//  1) Copied onto the stack,
	//  2) Modified with the 12-bit CSR address we are attempting to probe.
	//  3) Executed on the stack.
	.balign 4
	4:
	csrrc zero, 0x000, zero // Attempt CSR access
	jalr zero, 0x000(a4)   // Return

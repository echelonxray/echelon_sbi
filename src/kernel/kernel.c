#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./../progs/init/init.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./idle/idle_loop.h"
#include "./drivers/uart.h"
#include "./interrupts/interrupt.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./globals.h"
#include "./debug.h"

#define rck 2
#define srck 3
#define s_in 4

void kwrite(char* str) {
	uart_write((unsigned char*)str, UART0_BASE, strlen(str));
	return;
}

void read_buff(char* phys_mem, char* virt_mem, int spin) {
  kwrite("Read Buff\n");
  
  char buff[16];
  volatile uint64_t* ptr = 0;
  uintRL_t var;
  uint32_t number32;
	
	ptr = (uint64_t*)phys_mem;
	var = *ptr;
	kwrite("1 phys: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	ptr = (uint64_t*)virt_mem;
	var = *ptr;
	kwrite("1 virt: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	ptr = (uint64_t*)phys_mem;
	*ptr = 1;
	
	ptr = (uint64_t*)phys_mem;
	var = *ptr;
	kwrite("2 phys: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	ptr = (uint64_t*)virt_mem;
	var = *ptr;
	kwrite("2 virt: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	ptr = (uint64_t*)virt_mem;
	*ptr = 2;
	
	ptr = (uint64_t*)phys_mem;
	var = *ptr;
	kwrite("3 phys: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	ptr = (uint64_t*)virt_mem;
	var = *ptr;
	kwrite("3 virt: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	while (spin) {}
	
	return;
}

void int_handle() {
  kwrite("int_handle()\n");
  uintRL_t var;
  uint32_t var_lw;
  uint32_t var_hi;
  char buff[16];
  
  __asm__ __volatile__ ("csrr %0, mcause" : "=r" (var) : );
  var_lw = (uint32_t)(var >>  0);
  var_hi = (uint32_t)(var >> 32);
  
  kwrite("mcause:  0x");
  itoa(var_hi, buff, 16, -16, 8);
  kwrite(buff);
  itoa(var_lw, buff, 16, -16, 8);
  kwrite(buff);
  kwrite("\n");
  
  __asm__ __volatile__ ("csrr %0, mstatus" : "=r" (var) : );
  var_lw = (uint32_t)(var >>  0);
  var_hi = (uint32_t)(var >> 32);
  
  kwrite("mstatus: 0x");
  itoa(var_hi, buff, 16, -16, 8);
  kwrite(buff);
  itoa(var_lw, buff, 16, -16, 8);
  kwrite(buff);
  kwrite("\n");
  
  __asm__ __volatile__ ("csrr %0, mepc" : "=r" (var) : );
  var_lw = (uint32_t)(var >>  0);
  var_hi = (uint32_t)(var >> 32);
  
  kwrite("mepc:    0x");
  itoa(var_hi, buff, 16, -16, 8);
  kwrite(buff);
  itoa(var_lw, buff, 16, -16, 8);
  kwrite(buff);
  kwrite("\n");
  
  while (1) {}
  return;
}

signed int kmain(unsigned int argc, char* argv[], char* envp[]) {
	// START: CPU Init
	volatile uint32_t* ctrl_reg;
	
	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;
	
	char* s = "Hello, World!\n";
	
	//register uintRL_t var;
	//__asm__ __volatile__ ("csrw satp, %0" : : "r" (var));
	//__asm__ __volatile__ ("csrr %0, satp" : "r" (var) : );
	/*
	char* page_table_1 = 0;
	char* page_table_2 = 0;
	char* page_table_3 = 0;
	char* phys_data = 0;
	char* virt_data = 0;
	page_table_1 = 0x80020000ul;
	page_table_2 = 0x80021000ul;
	page_table_3 = 0x80022000ul;
	*/
	
	uintRL_t* ptr = 0;
	ptr = 0x80020000ul;
	//strcpy(ptr, s);
	
  uintRL_t numberRL;
  uint32_t number32;
  char buff[16];
  
  uintRL_t i = 0;
  //goto ssm;
  while (i < 8) {
    kwrite("Iteration: ");
    itoa(i, buff, 16, 10, 0);
    kwrite(buff);
    kwrite("\n");
    
    numberRL = *ptr;
    number32 = (uint32_t)numberRL;
    itoa(number32, buff, 16, 10, 0);
    ptr++;
    kwrite("mvendorid: ");
    kwrite(buff);
    kwrite("\n");
    
    numberRL = *ptr;
    number32 = (uint32_t)numberRL;
    itoa(number32, buff, 16, 10, 0);
    ptr++;
    kwrite("marchid: ");
    kwrite(buff);
    kwrite("\n");
    
    numberRL = *ptr;
    number32 = (uint32_t)numberRL;
    itoa(number32, buff, 16, 10, 0);
    ptr++;
    kwrite("mimpid: ");
    kwrite(buff);
    kwrite("\n");
    
    numberRL = *ptr;
    number32 = (uint32_t)numberRL;
    itoa(number32, buff, 16, 10, 0);
    ptr++;
    kwrite("mhartid: ");
    kwrite(buff);
    kwrite("\n");
    
    kwrite("\n");
    i++;
  }
  
	// Set Root Page Table @ 0x80021000;
	memset((void*)0x80021000ul, 0, 4096);
	memset((void*)0x80022000ul, 0, 4096);
	memset((void*)0x80023000ul, 0, 4096);
	memset((void*)0x80024000ul, 0, 4096);
	memset((void*)0x80025000ul, 0, 4096);
	memset((void*)0x80026000ul, 0, 4096);
	memset((void*)0x80027000ul, 0, 4096);
	uint64_t* pte_tb1 = (uint64_t*)0x80021000ul;
	uint64_t* pte_tb2 = (uint64_t*)0x80022000ul;
	uint64_t* pte_tb3 = (uint64_t*)0x80023000ul;
	uint64_t* pte_tb4 = (uint64_t*)0x80026000ul;
	uint64_t* pte_tb5 = (uint64_t*)0x80027000ul;
	char* phys_mem = (char*)0x80024000ul;
	char* virt_mem = (char*)0x80025000ul;
	uintRL_t phys_mem_loc = (uintRL_t)phys_mem;
	uintRL_t virt_mem_loc = (uintRL_t)virt_mem;
	uintRL_t var = 0x8000000000080021ul;
	__asm__ __volatile__ ("csrw satp, %0" : : "r" (var) );
	__asm__ __volatile__ ("csrr %0, mstatus" : "=r" (var) : );
	pte_tb1[(virt_mem_loc >> 30) & 0x1FF] = (( (uintRL_t)pte_tb2 & 0x00FFFFFFFFFFF000) >> 2) | 0x1; // 0x003F_FFFF_FFFF_FC00
	pte_tb2[(virt_mem_loc >> 21) & 0x1FF] = (( (uintRL_t)pte_tb3 & 0x00FFFFFFFFFFF000) >> 2) | 0x1;
	pte_tb3[(virt_mem_loc >> 12) & 0x1FF] = (((uintRL_t)phys_mem & 0x00FFFFFFFFFFF000) >> 2) | 0xF;
	pte_tb1[(phys_mem_loc >> 30) & 0x1FF] = (( (uintRL_t)pte_tb2 & 0x00FFFFFFFFFFF000) >> 2) | 0x1; // 0x003F_FFFF_FFFF_FC00
	pte_tb2[(phys_mem_loc >> 21) & 0x1FF] = (( (uintRL_t)pte_tb3 & 0x00FFFFFFFFFFF000) >> 2) | 0x1;
	pte_tb3[(phys_mem_loc >> 12) & 0x1FF] = (((uintRL_t)phys_mem & 0x00FFFFFFFFFFF000) >> 2) | 0xF;
	i = 0x80000000ul;
	while (i < 0x80021000ul) {
	  pte_tb3[(i >> 12) & 0x1FF] = ((i & 0x00FFFFFFFFFFF000) >> 2) | 0xF;
	  i += 1000ul;
	}
	virt_mem_loc = UART0_BASE;
	pte_tb1[(virt_mem_loc >> 30) & 0x1FF] = (( (uintRL_t)pte_tb4 & 0x00FFFFFFFFFFF000) >> 2) | 0x1;
	pte_tb4[(virt_mem_loc >> 21) & 0x1FF] = (( (uintRL_t)pte_tb5 & 0x00FFFFFFFFFFF000) >> 2) | 0x1;
	pte_tb5[(virt_mem_loc >> 12) & 0x1FF] = (((uintRL_t)UART0_BASE & 0x00FFFFFFFFFFF000) >> 2) | 0xF;
	__asm__ __volatile__ ("sfence.vma");
	
	kwrite("mstatus: 0x");
	number32 = (uint32_t)(var >> 32);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	number32 = (uint32_t)(var >> 0);
	itoa(number32, buff, 16, -16, 8);
	kwrite(buff);
	kwrite("\n");
	
	kwrite("\n");
	
	read_buff(phys_mem, virt_mem, 0);
	volatile uint64_t* ptr0 = 0;
	ptr0 = phys_mem;
	*ptr0 = 0;
	ptr0 = virt_mem;
	*ptr0 = 0;
	
	//ssm:
	
	kwrite("\nSwitching to Supervisor Mode\n\n");
	
	__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" (0x003FFFFFFFFFFFFFul));
	__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" (0x000000000000000Ful));
	
	__asm__ __volatile__ ( "mv a0, %0        \n"
	                       "mv a1, %1        \n"
	                       "csrw mepc, %2    \n"
	                       "csrw mtvec, %3   \n"
	                       "li a2,  1        \n"
	                       "slli a2, a2, 11  \n"
	                       "csrs mstatus, a2 \n"
	                       "li a2, 0x80      \n"
	                       "csrs mstatus, a2 \n"
	                       "li a2,  1        \n"
	                       "mret             \n"
	                       : 
	                       : "r" (phys_mem_loc), "r" (virt_mem), "r" (&read_buff), "r" (&int_handle)
	                       : "a0", "a1", "a2" );
	
	kwrite("\n");
	DEBUG_print("TraceY\n");
	CPU_WAIT();
	DEBUG_print("TraceZ\n");
	
	return 0;
}

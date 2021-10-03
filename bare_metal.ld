/*
This is not actually needed (At least not on actual hardware.), but 
it explicitly sets the entry point in the .elf file to be the same 
as the true entry point to the program. The global symbol my_entry_pt
is located at the start of src/kernel/entry.s.  More on this below.
*/
ENTRY(my_entry_pt)

MEMORY
{
	/*
	The SPI flash memory is mapped starting at memory address 
	0x2000_0000 in the FE310-G002 SOC.  This is the reset vector.
	*/
	flash (rx)  : ORIGIN = 0x80000000, LENGTH = 0x0000FFFF
	
	/*
	The actual ram onboard the SOC is mapped starting at memory
	address 0x8000_0000.
	*/
	ram   (rwx) : ORIGIN = 0x80010000, LENGTH = 0x0000FFFF
}

SECTIONS
{
	/*
	Force the linker to starting at address 0x2000_0000
	*/
	. = 0x80000000;
	
	/*
	Inform the linker that .text sections will be starting 
	at address 0x2000_0000
	*/
	.text : {
		/*
		Make sure the .text section from src/kernel/entry.o is 
		linked first.  The .text section of src/kernel/entry.s 
		is the actual entry machine code for the kernel and is 
		first in the file.  This way, at reset, exection starts 
		by jumping to this machine code.
		*/
		src/kernel/kstart_entry.o (.text);
		
		/*
		Link the rest of the kernel's .text sections.
		*/
		*.o (.text);
	} > flash
	
	/*
	Put in the .rodata in the flash after the actual machine code.
	*/
	.rodata : {
		*.o (.rodata);
		*.o (.rodata.*);
		*.o (.srodata);
		*.o (.srodata.*);
	} > flash
	
	/*
	section_CPIO_ARCHIVE : ALIGN(0x4) {
		USERSPACE_ARCHIVE_START = .;
		userspace.cpio.elf (section_CPIO_ARCHIVE);
	} > flash
	*/
	
	/*
	Make sure that global variables are mapped to reside starting 
	at the beginning of the actual ram on the SOC.
	*/
	. = 0x80010000;
	.sbss : {
		*.o (.sbss);
	} > ram
	.bss : {
		*.o (.bss);
	} > ram
	section_KHEAP_START (NOLOAD) : ALIGN(0x4) {
		/*
		At the very end of the space reserved for global variables 
		in the ram, link in this custom section.  This is used to
		add a symbol called KHEAP_START to the program that will 
		inform the C code where the heap can start.  This allows the 
		heap to start right after the global variables.
		*/
		KHEAP_START = .;
		/* src/kernel/entry.o (section_KHEAP_START); */
	} > ram
	
	/*
	Discard everything that hasn't be explictly linked.  I don't
	want the linker to guess where to put stuff.  If it doesn't know, 
	don't include it.  If this casues a linking error, good.  I want 
	to know that I need to fix something, rather than a silent failure 
	that could cause hard to debug issues later.  For instance, 
	without explicitly setting the .sbss and .bss sections above, 
	the linker attempted to put my global variables after the 
	machine code in the flash.  This would mean that ever access to 
	those variables would mean read a write to the external SPI flash 
	IC on real hardware.  I do not believe that initialized globals 
	are possible since there is nothing to initialize them.  So I don't
	want to, for instance, include the .data section.
	*/
	/DISCARD/ : {
		* (.*);
	}
}

/*
This is not actually needed (At least not on actual hardware.), but 
it explicitly sets the entry point in the .elf file to be the same 
as the true entry point to the program. The global symbol my_entry_pt
is located in src/kernel/kstart_entry.s.  More on this below.
*/
ENTRY(my_entry_pt)

MEMORY
{
	/*
	The actual RAM (DDR) is mapped starting at memory:   0x0000_0000_8000_0000.
	There are 16GB onboard the SiFive Unmatched.  16GB = 0x0000_0004_0000_0000.
	*/
	ram   (rwx) : ORIGIN = 0x0000000080000000, LENGTH = 0x0000000400000000
}

SECTIONS
{
	/*
	Inform the linker to start at the beginning of RAM
	*/
	. = ORIGIN(ram);
	
	/*
	Inform the linker that .text section will be linked first
	*/
	.text : {
		/*
		Make sure the .text section from src/kernel/kstart_entry.s is
		linked first.  The .text section of src/kernel/kstart_entry.s
		is the actual entry machine code for the kernel and should be at
		the start start of the file.  This way, at reset, execution
		starts by jumping to this machine code.
		*/
		src/kernel/kstart_entry.o (.text);
		
		/*
		Link the rest of the kernel's .text sections.
		*/
		*.o (.text);
	} > ram
	
	/*
	Put the .rodata section after the actual machine code.
	*/
	.rodata : {
		*.o (.rodata);
		*.o (.rodata.*);
		*.o (.srodata);
		*.o (.srodata.*);
	} > ram
	
	/*
	Next, make room for the global variables in RAM.
	*/
	.sbss : {
		*.o (.sbss);
	} > ram
	.bss : {
		*.o (.bss);
	} > ram
	
	/*
	At the very end of the space reserved for global variables
	in the ram, link in this custom section.  This is used to
	add a symbol called KHEAP_START to the program that will
	inform the C code where the heap can start.  This allows the
	heap to start right after the global variables on 128-bit
	alignment.  This ensures that data accesses will be aligned
	in 32-bit, 64-bit, and possible future 128-bit modes.
	*/
	section_KHEAP_START (NOLOAD) : ALIGN(0x10) {
		KHEAP_START = .;
	} > ram
	
	/*
	Discard everything that hasn't be explicitly linked.  I don't
	want the linker to guess where to put stuff.  If it doesn't know, 
	don't include it.  If this causes a linking error, good!  I want
	to know that I need to fix something, rather than a silent failure 
	that could cause hard to debug issues later.  For instance, 
	without explicitly setting the .sbss and .bss sections above, 
	the linker attempted to put my global variables after the 
	machine code in the flash when I was working on a FE310-G002 SOC
	with a different linker setup.  This would have meant that every
	access to those variables would result in a read or a write to the
	memory-mapped	external SPI flash IC on real hardware.  I also do
	not believe that initialized global variables are possible since
	there is nothing to initialize them.  So I don't want to, for
	instance, include the .data section.  If I get a linking error
	regarding that section, I will know to look for a bug in my code
	where I may have mistakenly used an initialized global variable.
	*/
	/DISCARD/ : {
		* (.*);
	}
}

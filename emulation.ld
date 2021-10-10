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
	PROGAMIMAGE_START = .;
	
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
		*.o (entry_text);
		
		/*
		Link the rest of the kernel's .text sections.
		*/
		*.o (.text);
	} > ram
	
	/*
	Create a TLS data block image
	*/
	section_THREAD_INIT_tdata : {
		THI_START = .;
		THI_tdata_START = .;
		*.o (.tdata);
		THI_tdata_END = .;
	} > ram
	section_THREAD_INIT_tbss (NOLOAD) : {
		THI_tbss_START = .;
		*.o (.tbss);
		THI_tbss_END = .;
		THI_END = .;
	} > ram
	
	.rodata : {
		/*
		THIS SHOULD PROBABLY BE MOVED IF SPLITING .rodata AWAY FROM
		.bss, ETC...  THEREFORE, gp CAN BE USED TO REFERENCE THAT
		DATA INSTEAD.
		
		Setup the gp register.  The linker assumes that	if this symbol is
		defined, that its location is also the value stored in the global
		pointer register, gp A.K.A. x3.  It will then relax references to 
		memory addresses within +0x7FF / -0x800 of gp.  The range is set
		like this because it the maximum range of the 12-bit signed offset
		that will	fit	into the of encoding register offset addressing.
		*/
		__global_pointer$ = . + 0x800;
		
		*.o (.rodata);
		*.o (.rodata.*);
		*.o (.srodata);
		*.o (.srodata.*);
	} > ram
	
	/*
	Next, make room for the global variables in RAM.
	*/
	INIT_DATA_PROGAMIMAGE_START = .;
	
	/*
	IF THE PROGRAM IS SPLIT INTO ROM AND RAM,
	SET THE VALUE OF "." TO THE RAM HERE.
	*/
	
	INIT_DATA_RUNTIME_START = .;
	.sdata : {
		*.o (.sdata);
	} > ram
	.data : {
		*.o (.data);
	} > ram
	INIT_DATA_RUNTIME_END = .;
	.sbss : {
		*.o (.sbss);
	} > ram
	.bss : {
		*.o (.bss);
	} > ram
	
	/*
	At the very end of the space reserved for global variables
	in the ram, link in these custom sections.  This is used to
	add a symbol called KHEAP_START to the program that will
	inform the code where the heap can start.  It is also used
	to add a symbol called KISTACK_TOP to the program that will
	inform the code where the top of the initial stack is
	(inclusive).  It is	on a 128-bit alignment.  This ensures
	that data accesses will be aligned	in 32-bit, 64-bit, and
	possible future 128-bit modes.
	*/
	section_FILLSPACE (NOLOAD) : ALIGN(0x10) {
		KISTACK_BOTTOM = .;
		FILL(0xDEADBEEF);
		. = KISTACK_BOTTOM + 0x1000;
		KISTACK_TOP = .;
		FILL(0xDEADBEEF);
		. = KISTACK_TOP + 0x10;
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

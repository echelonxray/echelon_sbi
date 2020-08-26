ENTRY(my_entry_pt)

MEMORY
{
	flash (rx)  : ORIGIN = 0x20000000, LENGTH = 0x1FFFFFFF
	ram   (rwx) : ORIGIN = 0x80000000, LENGTH = 0x00003FFF
}

SECTIONS
{
	. = 0x20000000;
	
	.text : {
		src/kernel/entry.o (.text);
		*.o (.text);
	} > flash
	.rodata : {
		*.o (.rodata);
		*.o (.rodata.*);
	} > flash
	
	. = 0x80000000;
	.sbss : {
		*.o (.sbss);
	} > ram
	.bss : {
		*.o (.bss);
	} > ram
	
	/DISCARD/ : {
		* (.*);
	}
}

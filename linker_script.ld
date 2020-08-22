ENTRY(my_entry_pt)

MEMORY
{
	flash (rx)  : ORIGIN = 0x20000000, LENGTH = 0x1FFFFFFF
	/* flash (rx)  : ORIGIN = 0x20400000, LENGTH = 0x1FFFFFFF */
	ram   (rwx) : ORIGIN = 0x80000000, LENGTH = 0x00003FFF
}

SECTIONS
{
	. = 0x20000000;
	/* . = 0x20400000; */
	
	.text : {
		src/kernel/entry.o (.text);
	} > flash
	
	/*
	Comment Out
	
	/DISCARD/ : {
		* (.*);
	}
	*/
}

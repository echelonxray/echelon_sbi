ENTRY(my_entry_pt)

SECTIONS
{
	. = 0x20000000;
	
	.text : {
		*.o (.text);
	}
	
	.rodata : {
		__global_pointer$ = . + 0x700;
		
		*.o (.rodata);
	}
	
	/DISCARD/ : {
		* (.*);
	}
}

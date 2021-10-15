ENTRY(my_entry_pt)

SECTIONS
{
	. = 0x80200000;
	
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

MEMORY
{
	flash (rx)  : ORIGIN = 0x20000000, LENGTH = 0x1FFFFFFF
	ram   (rwx) : ORIGIN = 0x80000000, LENGTH = 0x00003FFF
}

SECTIONS
{
	. = 0x20010000;
	
	.text : {
		hello_world_entry.o (.text);
	}
	
	/DISCARD/ : { * (.*); }
}

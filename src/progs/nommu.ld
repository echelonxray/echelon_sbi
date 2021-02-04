/*
This is a shared linker script used for the building of the 
various userspace programs under this directory.
*/

ENTRY(_start)

SECTIONS
{
  . = 0x10000;
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .got : { *(.got) }
  .got.plt : { *(.got.plt) }
  .data : { *(.data) *(.sdata) }
  .bss : { *(.bss) *(.sbss) }
}

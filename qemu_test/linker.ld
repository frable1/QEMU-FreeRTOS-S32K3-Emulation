/* file: linker.ld 
 * 
 * description: This file is the linker script for the s32k3x8evb board, for the qemu-arm
 * emulator. This file is used to define the memory regions of the board, this is a simple linker,
 * it only defines the memory regions for the text, data, bss and stack sections.
 */

/* define the memory regions */
/* ENTRY directive defines the entry point of the program */
ENTRY(_start)

/* SECTIONS directive defines the memory regions of the program */
SECTIONS
{
    /* text section (code)*/
    .text : { *(.text*) } 		
    /* data section, initialized variables */
    .data : { *(.data) }
    /* bss section, uninitialized variables */
    .bss : { *(.bss*) }
    /* stack section */
    /* The stack is placed at the end of the RAM */
    stack_top = 0x204bffff;
}



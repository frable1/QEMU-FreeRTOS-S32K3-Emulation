/*
 * Linker script for S32K3X8EVB with FreeRTOS, starting from 'mps2_m3.ld'
 * configured to use the flash and SRAM regions defined in the board code in "/qemu/include/hw/arm/s32k3x8_soc.h"
 */

/* memory regions */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00400000, LENGTH = 8M    /* Flash code memory: 8MB */
    SRAM (rwx)  : ORIGIN = 0x20400000, LENGTH = 768K  /* SRAM: 768KB */
    ITCM (rwx)  : ORIGIN = 0x00000000, LENGTH = 128K  /* ITCM: 128KB */
    DTCM (rwx)  : ORIGIN = 0x20000000, LENGTH = 256K  /* DTCM: 256KB */
}
ENTRY(Reset_Handler)

_Min_Heap_Size = 0x8 ;        /* Not used as building heap_4.c */
_Min_Stack_Size = 0x400 ;       /* Required amount of stack.  Used by main(), then re-used as the interrupt stack after the kernel starts. */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);


SECTIONS
{
    /* Interrupt Vector Table */
    .isr_vector :
    {
        KEEP(*(.isr_vector))    /* Ensure that the interrupt vector table is kept in FLASH memory */
    } > FLASH                   /* The vector table is placed in FLASH because it is read-only and needs to be at the start of the program */

    /* Code and read-only data */
    .text :
    {
        *(.text)            /* All code sections go here (including functions, etc.) */
        *(.rodata*)         /* Read-only data, such as constants, stored in FLASH */

    /*    
     *  *(.constdata*)      // removed since reduntant with *(.rodata*)
     *  _etext = .;         // useless
     *  _sidata = .;        // useless 
     */  

        KEEP(*(.init))      /* Keep the initialization code that runs at startup */
    } > FLASH               /* Code and read-only data are stored in FLASH as they do not change during execution */

    /* Initialized data section */
    .data :
    {
        _sdata = .;         /* Start address of the initialized data section */
        *(.data*)           /* Place all initialized variables here */
        _edata = .;         /* End address of the initialized data section */
    } > SRAM AT > FLASH     /* The .data section is loaded from FLASH but will reside in SRAM during runtime */


    /* Uninitialized data (BSS) */
    .bss :
    {
        _sbss = .;          /* Start address of the uninitialized data (BSS) section */
        *(.bss*)            /* Place all zero-initialized variables here */
        *(COMMON)           /* Include common variables that may not be initialized */
        _ebss = .;          /* End address of the uninitialized data (BSS) section */
    } > SRAM                /* The BSS section is placed in SRAM since it holds variables that are initialized to zero at runtime */


    /* Heap section (just before the stack) */
    ._user_heap (NOLOAD) :
    {
        . = ALIGN(8);                    /* Ensure heap section starts at an 8-byte aligned address */
        _sheap = .;                       /* Start of heap section (_sheap) */
        . += 0x20000;                     /* Allocate space for the heap (128 KB here, adjustable) */
        _eheap = .;                       /* End of heap section (_eheap) */
    } > SRAM  /* The heap is located in SRAM just below the stack to allow dynamic memory allocation during runtime */


    /* Stack section (top of SRAM) */
    ._user_stack (NOLOAD) :
    {
        . = ORIGIN(SRAM) + LENGTH(SRAM) - 0x4000;
        _estack = .;                /* Set the initial stack pointer (_estack) to the current location */
    } > SRAM                        /* The stack is located in SRAM because it will change dynamically during runtime */



    /* End of memory layout */
    _end = .;           /* Marks the end of the program's memory layout */

}

/* Define entry point */
ENTRY(Reset_Handler)

/* Provide stack information */
PROVIDE(_stack = _estack);


  


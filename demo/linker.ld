/*
 * Linker script for S32K3X8EVB with FreeRTOS, starting from 'mps2_m3.ld'
 * Configured to use the Flash, SRAM, ITCM and DTCM memory regions defined in the board code in "/qemu/include/hw/arm/s32k3x8_soc.h"
 */

/* Memory regions (Pag. 15 S32K3MemoriesGuide.pdf) */
/* ITCM and DTCM are used because are better for Real Time Operating Systems like FreeRTOS (Pag. 15 S32K3MemoriesGuide.pdf) */
MEMORY
{
    /* ITCM: Queues and performance-critical data */
    ITCM0 (RWX) : ORIGIN = 0x00000000, LENGTH = 64K
    ITCM2 (RWX) : ORIGIN = 0x00010000, LENGTH = 64K

    /* PFLASH: Main program */
    PFLASH (RX)  : ORIGIN = 0x00400000, LENGTH = 8M

    /* DFLASH: Non-volatile data */
    DFLASH (RW)  : ORIGIN = 0x10000000, LENGTH = 128K

    /* DTCM: Stack, heap, and critical data */
    DTCM0 (RW)  : ORIGIN = 0x20000000, LENGTH = 128K
    DTCM2 (RW)  : ORIGIN = 0x21800000, LENGTH = 128K

    /* SRAM: Generic data */
    SRAM_STDBY (RW)  : ORIGIN = 0x20400000, LENGTH = 64K
    SRAM0 (RW)  : ORIGIN = 0x20410000, LENGTH = 192K
    SRAM1 (RW)  : ORIGIN = 0x20440000, LENGTH = 256K
    SRAM2 (RW)  : ORIGIN = 0x20480000, LENGTH = 256K

    /* Total SRAM region (without SRAM_STDBY) [768 KB - 64 KB = 704 KB]*/
    SRAM (RW)  : ORIGIN = 0x20400000, LENGTH = 704K

    /* Flash UTEST */
    UTEST (RX)  : ORIGIN = 0x1B000000, LENGTH = 8K
}

_Min_Heap_Size = 0x8 ;     /* Not used as building heap_4.c */
_Min_Stack_Size = 0x400 ;  /* Required amount of stack. Used by main(), then re-used as the interrupt stack after the kernel starts. */

/* Initial stack pointer */
_estack = ORIGIN(DTCM2) + LENGTH(DTCM2);

/* Linker Sections */
SECTIONS
{
    /* ISR Vector Table */
    .isr_vector :
    {
        __vector_table = .;
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > ITCM0

    /* Read-only code and data section */
    .text :
    {
        *(.text*)         /* Code */
        *(.rodata*)       /* Read-only data */
        *(.constdata*)    /* Constant data */
        _etext = .;       /* End of text section */
        KEEP(*(.init))    /* Initialization code */
    } > PFLASH

    /* Initialized data (RAM) */
    .data :
    {
        . = ALIGN(8);
        _sdata = .;       /* Start of initialized data in RAM */
        *(.data)          /* Initialized global/static variables */
        *(.data.*)        /* Section-specific initialized data */
        *(vtable)         /* Vector table */
        _edata = .;       /* End of initialized data in RAM */
    } > DTCM0

    /* Uninitialized data (RAM) */
    .bss :
    {
        . = ALIGN(8);
        _sbss = .;        /* Start of uninitialized data */
        *(.bss)           /* Uninitialized global/static variables */
        *(.bss.*)         /* Section-specific uninitialized data */
        _ebss = .;        /* End of uninitialized data */
    } > DTCM0

    /* Standby RAM */
    .standby_ram :
    {
        . = ALIGN(8);
        *(.standby_ram)   /* Data specific to standby mode */
    } > SRAM_STDBY

    /* Test data */
    .utest :
    {
        KEEP(*(.utest))   /* Test-related data */
    } > UTEST

    /* Heap */
    .heap :
    {
        . = ALIGN(8);
        PROVIDE(end = .);         /* End of all sections */
        PROVIDE(_end = .);
        _heap_bottom = .;         /* Start of heap */
        . = . + _Min_Heap_Size;   /* Allocate minimum heap size */
        _heap_top = .;            /* End of heap */
    } > DTCM0

    /* Stack */
    .stack :
    {
        . = ALIGN(8);
        __stack_start__ = .;      /* Start of stack */
        . = . + _Min_Stack_Size;  /* Allocate minimum stack size */
        __stack_end__ = .;        /* End of stack */
    } > DTCM2
    
    /* End of memory layout */
    _end = .; /* Marks the end of the program's memory layout */
}

/* Entry point */
ENTRY(Reset_Handler)

/* Initial stack pointer */
PROVIDE(_stack = _estack);

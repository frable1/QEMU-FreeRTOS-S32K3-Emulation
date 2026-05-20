#ifndef ARM_S32K3X8_SOC_H
#define ARM_S32K3X8_SOC_H

#include "hw/or-irq.h"
#include "hw/arm/armv7m.h"
#include "hw/clock.h"
#include "qom/object.h"

#define TYPE_S32K3X8_SOC "s32k3x8-soc"
OBJECT_DECLARE_SIMPLE_TYPE(S32K3X8SOCState, S32K3X8_SOC)

// Pag. 962 Reference Manual
#define SYSCLK_FRQ 240000000ULL                     // 240 MHz 
#define REFCLK_FRQ 1000000ULL                       // 1 MHz
#define AIPS_PLAT_CLK_FRQ 120000000ULL              // 120 MHz
#define AIPS_SLOW_CLK_FRQ 60000000ULL               // 60 MHz

// Pag. 3 S32K3MemoriesGuide.pdf and pag. 774 Reference Manual
#define FLASH_CODE_BASE_ADDRESS 0x00400000          // Flash code memory base address
#define FLASH_CODE_SIZE (8192 * 1024)               // Flash code memory size (8 MB)
#define FLASH_DATA_BASE_ADDRESS 0x10000000          // Flash data memory base address
#define FLASH_DATA_SIZE (128 * 1024)                // Flash data memory size (128 KB)
#define UTEST_BASE_ADDRESS 0x1B000000               // UTEST memory base address
#define UTEST_SIZE (8 * 1024)                       // UTEST memory size (8 KB)

// Pag. 13 S32K3MemoriesGuide.pdf
#define ITCM0_BASE_ADDRESS 0x00000000               // ITCM0 base address
#define ITCM0_SIZE (64 * 1024)                      // ITCM0 size (64 KB)
#define ITCM2_BASE_ADDRESS 0x00010000               // ITCM2 base address
#define ITCM2_SIZE (64 * 1024)                      // ITCM2 size (64 KB)
#define DTCM0_BASE_ADDRESS 0x20000000               // DTCM0 base address
#define DTCM0_SIZE (128 * 1024)                     // DTCM0 size (128 KB)
#define DTCM2_BASE_ADDRESS 0x21800000               // DTCM2 base address
#define DTCM2_SIZE (128 * 1024)                     // DTCM2 size (128 KB)
#define SRAM_STDBY_BASE_ADDRESS 0x20400000          // SRAM Standby base address
#define SRAM_STDBY_SIZE (64 * 1024)                 // SRAM Standby size (64 KB)
#define SRAM0_BASE_ADDRESS 0x20410000               // SRAM Block0 base address
#define SRAM0_SIZE (192 * 1024)                     // SRAM Block0 size (192 KB) [Total = 256 KB (Standby + Sram0)]
#define SRAM1_BASE_ADDRESS 0x20440000               // SRAM Block1 base address
#define SRAM1_SIZE (256 * 1024)                     // SRAM Block1 size (256 KB)
#define SRAM2_BASE_ADDRESS 0x20480000               // SRAM Block2 base address
#define SRAM2_SIZE (256 * 1024)                     // SRAM Block2 size (256 KB)

// Pag. 4609 Reference Manual
#define UART_0_7_BASE_ADDR 0x40328000               // UART from 0 to 7 base address
#define UART_8_15_BASE_ADDR 0x4048C000              // UART from 8 to 15 base address

struct S32K3X8SOCState {
    SysBusDevice parent_obj;
    
    ARMv7MState cpu;
    
    MemoryRegion flash_code;
    MemoryRegion flash_data;
    MemoryRegion utest;
    
    MemoryRegion itcm0;
    MemoryRegion itcm2;
    
    MemoryRegion dtcm0;
    MemoryRegion dtcm2;
    
    MemoryRegion sram_stdby;
    MemoryRegion sram0;
    MemoryRegion sram1;
    MemoryRegion sram2;
    
    Clock *sysclk;
    Clock *refclk;
    Clock *aips_plat_clk;
    Clock *aips_slow_clk;
};

#endif


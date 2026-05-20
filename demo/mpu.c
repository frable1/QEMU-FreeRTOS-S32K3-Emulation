#include "mpu.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>

/* MPU register block (System Control Space) */
#define MPU_BASE (0xE000ED90UL)
#define MPU_CTRL (*(volatile uint32_t *)(MPU_BASE + 0x04))
#define MPU_RNR  (*(volatile uint32_t *)(MPU_BASE + 0x08))
#define MPU_RBAR (*(volatile uint32_t *)(MPU_BASE + 0x0C))
#define MPU_RASR (*(volatile uint32_t *)(MPU_BASE + 0x10))

/* SCB registers for fault diagnostics */
#define SCB_SHCSR   (*(volatile uint32_t *)0xE000ED24UL)  /* System Handler Control and State */
#define SCB_CFSR    (*(volatile uint32_t *)0xE000ED28UL)  /* Configurable Fault Status */

/* size code n encodes size = 2^(n+1) bytes */
#ifndef SIZE_8MB
#define SIZE_8MB   22U  /* 2^(22+1) = 8 MB */
#endif
#ifndef SIZE_1MB
#define SIZE_1MB   19U  /* 2^(19+1) = 1 MB */
#endif
#ifndef SIZE_512MB
#define SIZE_512MB 28U  /* 2^(28+1) = 512 MB */
#endif


/*
 * MPU_init
 *  R0: Flash RO      8 MB @ 0x0040_0000 (aligned)
 *  R1: SRAM RW       1 MB @ 0x2040_0000
 *  R2: PERIPH RW   512 MB @ 0x4000_0000
 *  R3: TRAP NO-ACCESS 32 KB @ 0x2040_8000 (inside SRAM)  <-- used to force a fault
 */
void MPU_init(void)
{
    /* Disable MPU while configuring */
    MPU_CTRL = 0U;

    /* Region 0: flash RO (8 MB aligned at 0x00400000) */
    MPU_RNR  = 0U;
    MPU_RBAR = 0x00400000U;
    MPU_RASR = MPU_RASR_VALUE(SIZE_8MB, MPU_AP_RO);

    /* Region 1: SRAM RW (1 MB @ 0x20400000) */
    MPU_RNR  = 1U;
    MPU_RBAR = 0x20400000U;
    MPU_RASR = MPU_RASR_VALUE(SIZE_1MB, MPU_AP_RW);

    /* Region 2: Peripherals RW (512 MB @ 0x40000000) */
    MPU_RNR  = 2U;
    MPU_RBAR = 0x40000000U;
    MPU_RASR = MPU_RASR_VALUE(SIZE_512MB, MPU_AP_RW);

    /* Region 3: NO-ACCESS trap-zone (32 KB @ 0x20408000) */
    MPU_RNR  = 3U;
    MPU_RBAR = 0x20408000U;                /* must be 32KB-aligned: 0x....8000 */
    MPU_RASR = MPU_RASR_VALUE(SIZE_32KB, MPU_AP_NO);

    /* Enable MPU (bit0) + privileged default map (bit2) */
    MPU_CTRL = 0x5U;

    /* Barriers to ensure settings take effect */
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

/* Small helper to read-back and check a region */
static int mpu_check_region(uint32_t idx, uint32_t exp_rbar, uint32_t exp_size_n, uint32_t exp_ap)
{
    MPU_RNR = idx;
    const uint32_t rbar = MPU_RBAR;
    const uint32_t rasr = MPU_RASR;

    const int en_ok   = (rasr & 1U) != 0U;                     /* enabled */
    const int size_ok = ((rasr >> 1) & 0x1FU) == (exp_size_n & 0x1FU);
    const int ap_ok   = (rasr & (7U << 24)) == (exp_ap & (7U << 24));
    const int base_ok = (rbar & 0xFFFFFFE0U) == (exp_rbar & 0xFFFFFFE0U);

    return en_ok && size_ok && ap_ok && base_ok;
}


/*
 * MPU_selftest
 *  - Prints CTRL flags
 *  - Verifies regions 0..2 match MPU_init()
 *  - Functional SRAM RW probe
 */
void MPU_selftest(void)
{
    UART_printf("\n--- MPU Self-Test ---\n");

    const uint32_t ctrl = MPU_CTRL;
    const int mpu_enabled      = (ctrl & 0x1U) != 0U;
    const int privdefn_enabled = (ctrl & 0x4U) != 0U;

    UART_printf("CTRL: ");
    UART_printf(mpu_enabled ? "EN=1 " : "EN=0 ");
    UART_printf(privdefn_enabled ? "PRIVDEF=1\n" : "PRIVDEF=0\n");

    /* Region checks (must match MPU_init()) */
    const int r0_ok = mpu_check_region(0, 0x00400000U, SIZE_8MB, MPU_AP_RO);
    const int r1_ok = mpu_check_region(1, 0x20400000U, SIZE_1MB,   MPU_AP_RW);
    const int r2_ok = mpu_check_region(2, 0x40000000U, SIZE_512MB, MPU_AP_RW);

    UART_printf("Region 0 (Flash RO 8MB):   "); UART_printf(r0_ok ? "PASS\n" : "FAIL\n");
    UART_printf("Region 1 (SRAM RW 1MB):    "); UART_printf(r1_ok ? "PASS\n" : "FAIL\n");
    UART_printf("Region 2 (PERIPH RW 512MB):"); UART_printf(r2_ok ? "PASS\n" : "FAIL\n");

    /* Functional RW check in SRAM */
    volatile uint32_t *sram = (uint32_t *)0x20400000U;
    const uint32_t before = *sram;
    *sram = 0xA5A5A5A5U;
    const uint32_t after = *sram;
    const int sram_rw_ok = (after == 0xA5A5A5A5U);
    *sram = before; /* restore */

    UART_printf("SRAM RW test: "); UART_printf(sram_rw_ok ? "PASS\n" : "FAIL\n");

    const int all_ok = mpu_enabled && privdefn_enabled && r0_ok && r1_ok && r2_ok && sram_rw_ok;
    UART_printf(all_ok ? "MPU CONFIG: PASS\n" : "MPU CONFIG: FAIL\n");
    UART_printf("---------------------\n");
}


/*
 * MPU_fault_test
 *  - “Flash write” probe: on some QEMU models this might NOT trap.
 *    We print CFSR in case it doesn’t.
 */
void MPU_fault_test(void)
{
    /* Enable MemManage faults so MPU violations raise MemManage (not HardFault) */
    SCB_SHCSR |= (1u << 16);  /* MEMFAULTENA */

    UART_printf("\n--- MPU Fault Test: writing to Flash ---\n");

    volatile uint32_t *flash_ptr = (uint32_t *)0x00400000U; /* base Flash, RO by MPU */
    *flash_ptr = 0xDEADBEEFU;  /* expected to fault on real HW; may NOT fault in this QEMU */

    char buf[64];
    snprintf(buf, sizeof(buf), "CFSR=0x%08lX\n", (unsigned long)SCB_CFSR);
    UART_printf(buf);
    UART_printf("Unexpected: flash write did NOT fault (emulator quirk).\n");
}

/*
 * MPU_forced_fault
 *  - Deterministic MemManage: write into the NO-ACCESS trap zone.
 *  - Requires MemManage enabled and MemManage_Handler in vector table.
 */
void MPU_forced_fault(void)
{
    /* Enable MemManage faults so MPU violations raise MemManage (not HardFault) */
    SCB_SHCSR |= (1u << 16);  /* MEMFAULTENA */

    UART_printf("\n--- MPU Forced Fault Test (NO-ACCESS @0x20408000) ---\n");

    volatile uint32_t *trap = (uint32_t *)0x20408000U; /* inside region 3 NO-ACCESS */
    *trap = 0xDEADBEEFU;  /* Expected: MemManage fault, execution should not continue */

    /* If we get here, emulator did not trap (very unlikely with this method). */
    char buf[64];
    snprintf(buf, sizeof(buf), "CFSR=0x%08lX\n", (unsigned long)SCB_CFSR);
    UART_printf(buf);
    UART_printf("Unexpected: write in NO-ACCESS region did NOT fault.\n");
}

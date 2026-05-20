#ifndef __PRINTF__
#define __PRINTF__

#include "FreeRTOS.h"

#define LPUART_BASE_ADDR 0x40328000

#define LPUART_CR1   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x00))
#define LPUART_CR2   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x04))
#define LPUART_CR3   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x08))
#define LPUART_BRR   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x0C))
#define LPUART_GTPR  (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x10))
#define LPUART_RTOR  (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x14))
#define LPUART_RQR   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x18))
#define LPUART_ISR   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x1C))
#define LPUART_ICR   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x20))
#define LPUART_RDR   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x24))
#define LPUART_TDR   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x28))

// Flag for ISR register
#define TXE_FLAG  (1 << 7)  // TXE (Transmit Data Register Empty)

void UART_init(void);
void UART_printf(const char *s);

#endif


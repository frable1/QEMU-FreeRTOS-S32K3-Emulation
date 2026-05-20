#include "uart.h"

void UART_init(void) {
    // Configure the baud rate
    LPUART_BRR = 16;

    // Enable the transmitter and receiver
    LPUART_CR1 = (1 << 3) | (1 << 2); // TE (Transmitter enable) and RE (Receiver enable)

    // Enable the UART
    LPUART_CR1 |= (1 << 0); // UE (USART enable)
}

void UART_printf(const char *s) {
    while (*s != '\0') {
        while (!(LPUART_ISR & TXE_FLAG)) {
            // Wait for the transmission register to be empty
        }
        
        // Write the character to TDR
        LPUART_TDR = (unsigned int)(*s);
        s++;
    }
}


#include <stdint.h>
#include "uart.h"

int main(void) {
  UART_init();
  
  char msg[] = "Hello, World!\n";
  UART_printf(msg);
  
  return 0;
}


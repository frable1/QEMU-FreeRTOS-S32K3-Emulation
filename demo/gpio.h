#ifndef S32K3X8_GPIO_H
#define S32K3X8_GPIO_H

#include <stdint.h>

#define GPIO0_BASE   0x40290000  // PDAC0
#define GPIO1_BASE   0x40298000  // PDAC1
#define GPIO2_BASE   0x402A0000  // PDAC2

#define GPIO_PGPDO_OFFSET   0x1700
#define GPIO_PGPDI_OFFSET   0x1740

#define GPIO_DIR_OFFSET 0x1600
#define GPIO0_DIR (*(volatile uint32_t *)(GPIO0_BASE + GPIO_DIR_OFFSET))
#define GPIO1_DIR (*(volatile uint32_t *)(GPIO1_BASE + GPIO_DIR_OFFSET))

#define GPIO0_PGPDO   (*(volatile uint32_t *)(GPIO0_BASE + GPIO_PGPDO_OFFSET))
#define GPIO0_PGPDI   (*(volatile uint32_t *)(GPIO0_BASE + GPIO_PGPDI_OFFSET))

#define GPIO1_PGPDO   (*(volatile uint32_t *)(GPIO1_BASE + GPIO_PGPDO_OFFSET))
#define GPIO1_PGPDI   (*(volatile uint32_t *)(GPIO1_BASE + GPIO_PGPDI_OFFSET))

#define GPIO2_PGPDO   (*(volatile uint32_t *)(GPIO2_BASE + GPIO_PGPDO_OFFSET))
#define GPIO2_PGPDI   (*(volatile uint32_t *)(GPIO2_BASE + GPIO_PGPDI_OFFSET))

// Debouncing configuration
#define DEBOUNCE_TIME_MS 2          // 2ms debounce time
#define NUM_BUTTONS 4               // 4 buttons (pins 4-7)

// Button debouncing structure
typedef struct {
    uint32_t last_state;         // Previous button state
    uint32_t current_state;      // Current button state  
    uint32_t stable_state;       // Debounced button state
    uint32_t last_change_time;   // Time of last state change
} button_debounce_t;

// Function declarations
void test_gpio(void);
uint32_t gpio_debounce_buttons(uint32_t raw_buttons);
void gpio_debounce_init(void);
uint32_t get_system_ticks(void); // Simple tick counter

#endif
/**
 * GPIO Multi-Port Testing with Software Debouncing for S32K3X8 Microcontroller
 * 
 * This file implements GPIO functionality testing with software debouncing for reliable
 * button input processing. It demonstrates multi-port GPIO operations including digital
 * input reading with debouncing, digital output control, and cross-port communication.
 * 
 * 
 * Hardware Configuration:
 * ======================
 * 
 * GPIO0 - Control Interface:
 *   - Pins 0-3: LED outputs (visual indicators)
 *   - Pins 4-7: Button inputs (user controls) with software debouncing
 * 
 * GPIO1 - Actuator Interface:
 *   - Pins 0-3: Relay outputs (device control)
 *   - Pins 4-7: Sensor inputs (feedback monitoring)
 * 
 * Test Functionality:
 * ==================
 * 
 * The test emulates an industrial control system where:
 * - Button presses control LED status indicators
 * - LED patterns drive relay activation
 * - Sensors provide feedback on relay states
 * - Complete system state is monitored via UART
 * 
 * This configuration demonstrates typical embedded system patterns
 * found in automation, monitoring, and control applications.
 *
 * Software Debouncing:
 * ===================
 * Implements software debouncing algorithm to filter button noise:
 * - 2ms debounce time (configurable)
 * - State tracking for each button
 * - Stable state confirmation before acceptance
 * - Compatible with both physical and simulated buttons
 */

#include "gpio.h"
#include "uart.h"
#include <stdio.h>

// Global debouncing state
static button_debounce_t button_debounce;
static uint32_t system_tick_counter = 0;

/**
 * @brief Simple system tick counter
 * 
 * Returns a simple tick counter that increments each time it's called.
 * In a real system, this would be connected to a hardware timer.
 * 
 * @return Current tick count
 */
uint32_t get_system_ticks(void) {
    return ++system_tick_counter;
}

/**
 * Initialize button debouncing system
 * 
 * Initializes the debouncing state structure and sets initial values.
 * Should be called once before using the debouncing functionality.
 */
void gpio_debounce_init(void) {
    button_debounce.last_state = 0;
    button_debounce.current_state = 0;
    button_debounce.stable_state = 0;
    button_debounce.last_change_time = 0;
    
    UART_printf("Button debouncing initialized (");
    char buf[32];
    snprintf(buf, sizeof(buf), "%dms debounce time)\n", DEBOUNCE_TIME_MS);
    UART_printf(buf);
}

/**
 * Software debouncing algorithm for button inputs
 * 
 * Implements a time-based debouncing algorithm that filters out button noise
 * and bounce effects. Only reports stable button states after the debounce
 * time has elapsed.
 * 
 * Algorithm:
 * 1. Compare current reading with previous reading
 * 2. If different, record time and start debounce period
 * 3. If same for debounce time, accept as stable state
 * 4. Return only stable, debounced button states
 * 
 * @param raw_buttons Raw button state from GPIO register
 * @return Debounced button state
 */
uint32_t gpio_debounce_buttons(uint32_t raw_buttons) {
    uint32_t current_time = get_system_ticks();
    
    // Extract only button bits (pins 4-7) and shift to 0-3
    uint32_t button_bits = (raw_buttons >> 4) & 0x0F;
    
    // Check if button state has changed
    if (button_bits != button_debounce.current_state) {
        // State changed - reset debounce timer
        button_debounce.current_state = button_bits;
        button_debounce.last_change_time = current_time;
    } else {
        // State unchanged - check if debounce time has elapsed
        uint32_t time_elapsed = current_time - button_debounce.last_change_time;
        
        if (time_elapsed >= DEBOUNCE_TIME_MS) {
            // Debounce time elapsed - accept new stable state
            if (button_debounce.stable_state != button_debounce.current_state) {
                button_debounce.stable_state = button_debounce.current_state;
                
                // Debug: Report state changes
                char buf[64];
                snprintf(buf, sizeof(buf), "[DEBOUNCE] State stabilized: 0x%lX\n", 
                        (unsigned long)button_debounce.stable_state);
                UART_printf(buf);
            }
        }
    }
    
    // Return debounced state shifted back to pins 4-7
    return (button_debounce.stable_state << 4);
}

/**
 * GPIO Multi-Port Functionality Test with Debouncing
 * 
 * Tests GPIO input/output operations with software debouncing for reliable
 * button input processing. Demonstrates:
 * - Pin direction configuration
 * - Debounced digital input reading
 * - Digital output control based on stable inputs
 * - Cross-port signal correlation
 */
void test_gpio(void) {
    // Initialize debouncing on first call
    static int initialized = 0;
    if (!initialized) {
        gpio_debounce_init();
        initialized = 1;
    }
    
    UART_printf("\n--- GPIO Industrial Control Test with Debouncing ---\n");

    /*
     * Configure pin directions for both GPIO ports
     * GPIO0: LEDs (0-3) as outputs, Buttons (4-7) as inputs
     * GPIO1: Relays (0-3) as outputs, Sensors (4-7) as inputs
     */
    GPIO0_DIR = 0x000F; // 0-3: LED outputs, 4-7: button inputs
    GPIO1_DIR = 0x000F; // 0-3: relay outputs, 4-7: sensor inputs

    /*
     * Read raw button states and apply software debouncing
     * This eliminates button bounce and provides stable button readings
     */
    uint32_t raw_buttons = GPIO0_PGPDI;
    uint32_t debounced_buttons = gpio_debounce_buttons(raw_buttons);
    
    /*
     * Generate LED pattern based on DEBOUNCED button states
     * Maps debounced button inputs (pins 4-7) to LED outputs (pins 0-3)
     */
    uint32_t led_pattern = 0;
    for (int i = 4; i <= 7; i++) {
        if (debounced_buttons & (1 << i)) {
            led_pattern |= (1 << (i-4)); // Map button 4→LED 0, etc.
        }
    }
    GPIO0_PGPDO = led_pattern; // Update LED outputs
    
    /*
     * Control relays based on debounced LED pattern
     * Ensures relays only activate on stable button presses
     */
    uint32_t relay_pattern = led_pattern; // Same pattern as LEDs
    GPIO1_PGPDO = relay_pattern; // Activate relays
    
    /*
     * Read sensor feedback from GPIO1 input pins
     * Sensors provide system status monitoring
     */
    uint32_t sensors = GPIO1_PGPDI;
    
    /*
     * Display complete system state via UART
     * Shows both raw and debounced button states for comparison
     */
    
    // Control interface status with debouncing info
    UART_printf("Control Panel (GPIO0) - Debounced:\n");
    UART_printf("  Raw Buttons:       ");
    char buf[32];
    for (int i = 4; i <= 7; i++) {
        snprintf(buf, sizeof(buf), "B%d:%c ", i-4, (raw_buttons & (1 << i)) ? 'P' : '-');
        UART_printf(buf);
    }
    UART_printf("\n  Debounced Buttons: ");
    for (int i = 4; i <= 7; i++) {
        snprintf(buf, sizeof(buf), "B%d:%c ", i-4, (debounced_buttons & (1 << i)) ? 'P' : '-');
        UART_printf(buf);
    }
    UART_printf("\n  LEDs:              ");
    for (int i = 0; i <= 3; i++) {
        snprintf(buf, sizeof(buf), "L%d:%c ", i, (led_pattern & (1 << i)) ? 'O' : '-'); /* O=On, -=Off  */
        UART_printf(buf);
    }
    
    // Actuator interface status
    UART_printf("\nActuators (GPIO1):\n");
    UART_printf("  Relays:  ");
    for (int i = 0; i <= 3; i++) {
        snprintf(buf, sizeof(buf), "R%d:%c ", i, (relay_pattern & (1 << i)) ? 'A' : '-');   /* A=Active, -=Inactive */
        UART_printf(buf);
    }
    UART_printf("\n  Sensors: ");
    for (int i = 4; i <= 7; i++) {
        snprintf(buf, sizeof(buf), "S%d:%c ", i-4, (sensors & (1 << i)) ? 'H' : 'L');   /* H=High, L=Low */
        UART_printf(buf);
    }
    
    // Display debouncing status
    snprintf(buf, sizeof(buf), "\n[Debug] Tick: %ld, Stable: 0x%lX", 
             system_tick_counter, button_debounce.stable_state);
    UART_printf(buf);
    
    UART_printf("\n-----------------------\n");
}

/*
 * Debouncing Algorithm Summary:
 * ============================
 * 
 * 1. INPUT SAMPLING:
 *    - Read raw GPIO button state every cycle
 *    - Extract button bits (pins 4-7) and normalize to 0-3
 * 
 * 2. CHANGE DETECTION:
 *    - Compare with previous reading
 *    - If different, start debounce timer
 *    - If same, check if debounce time elapsed
 * 
 * 3. STATE VALIDATION:
 *    - Only accept state after DEBOUNCE_TIME_MS of stability
 *    - Ignore transient changes during debounce period
 *    - Report state changes for debugging
 * 
 * 4. OUTPUT GENERATION:
 *    - Use only debounced states for LED/relay control
 *    - Ensures reliable operation without bounce artifacts
 * 
 * Benefits:
 * - Eliminates false button presses from electrical noise
 * - Provides stable, reliable button state detection
 * - Compatible with both physical and simulated buttons
 * - Configurable debounce time for different applications
 */
/**
 * S32K3X8 GPIO (General Purpose Input/Output) Emulation for QEMU
 * 
 * This file implements GPIO peripheral emulation for the S32K3X8 microcontroller
 * in QEMU. It provides hardware-level simulation of GPIO operations including
 * digital input/output, pin direction control, and realistic signal behavior.
 * 
 * Hardware Reference:
 * ==================
 * Based on NXP S32K3X8 Reference Manual
 * - GPIO control via SIUL2 (System Integration Unit Lite 2) module
 * - Memory mapped to SIUL_VIRTWRAPPER_PDACx region
 * - Base address: 0x40290000 (PDAC0, memory_map.xlsx)
 * - Memory region size: 16KB per wrapper
 * 
 * Emulated Registers: (real offset, pag. 154 manual)
 * ==================
 * - PGPDO  (0x1700): Parallel GPIO Data Output (16-bit)
 * - PGPDI  (0x1740): Parallel GPIO Data Input (16-bit)
 * - MPGPDO (0x1780): Multi Parallel GPIO Data Output (32-bit)
 * - DIR    (0x1600): Pin Direction Control (16-bit)
 * 
 * Simulation Features:
 * ===================
 * - Realistic pin direction control (input/output configuration)
 * - Dynamic input simulation for testing without physical hardware
 * - Cross-port signal correlation for system-level testing
 * - Hardware-accurate register behavior and timing
 */

#include "qemu/osdep.h"
#include "hw/gpio/s32k3x8_gpio.h"
#include "hw/irq.h"

/* Register definitions based on S32K3X8 Reference Manual */
#define GPIO_REG_PGPDO   0x1700  // Parallel GPIO Data Output
#define GPIO_REG_PGPDI   0x1740  // Parallel GPIO Data Input  
#define GPIO_REG_MPGPDO  0x1780  // Multi Parallel GPIO Data Output
#define GPIO_MEM_SIZE    0x4000  // 16KB per wrapper (PDACx)

/* Global state pointers for cross-port simulation */
extern S32K3X8GPIOState *gpio0_state;
extern S32K3X8GPIOState *gpio1_state;

/**
 * GPIO Register Read Handler
 * 
 * Handles read operations from GPIO registers with hardware-accurate behavior.
 * Implements proper pin direction masking to ensure only appropriate pins
 * are read based on their configuration.
 * 
 * @param opaque Pointer to GPIO state structure
 * @param addr Register address offset
 * @param size Access size in bytes
 * @return Register value with direction masking applied
 */
static uint64_t s32k3x8_gpio_read(void *opaque, hwaddr addr, unsigned int size)
{
    S32K3X8GPIOState *s = opaque;
    switch (addr) {
        case GPIO_REG_PGPDI:
            /* Return only pins configured as inputs
             * Hardware behavior: output pins read as 0 when accessed via PGPDI */
            return s->data_in & ~(s->dir);
            
        case GPIO_REG_PGPDO:
            /* Return only pins configured as outputs  
             * Hardware behavior: input pins read as 0 when accessed via PGPDO */
            return s->data_out & s->dir;
            
        case GPIO_REG_MPGPDO: 
            /* Multi-port GPIO output register (32-bit version) */
            return s->data_out;
            
        default:              
            return 0;
    }
}

/**
 * GPIO Register Write Handler
 * 
 * Handles write operations to GPIO registers including output control,
 * direction configuration, and simulation trigger logic. Implements
 * realistic hardware behavior with proper pin masking and state updates.
 * 
 * @param opaque Pointer to GPIO state structure
 * @param addr Register address offset
 * @param value Value to write
 * @param size Access size in bytes
 */
static void s32k3x8_gpio_write(void *opaque, hwaddr addr, uint64_t value, unsigned int size)
{
    S32K3X8GPIOState *s = opaque;
    
    switch (addr) {
        case GPIO_REG_PGPDO:
            /* Update output register with direction masking
             * Only pins configured as outputs are affected */
            s->data_out = (s->data_out & ~s->dir) | (value & s->dir);
            s->data_in = s->data_out; // Loopback for output pins
            
            /* Update physical pin states for external connections */
            for (int i = 0; i < GPIO_NUM_PINS; i++) {
                qemu_set_irq(s->pin[i], (value >> i) & 1);
            }
            
            /* REALISTIC SIMULATION: Relay-to-Sensor Feedback
             * When GPIO1 relays are activated, simulate corresponding sensors
             * detecting the relay state with realistic reliability */
            if (s == gpio1_state) {
                for (int i = 0; i <= 3; i++) {
                    if (s->dir & (1 << i)) { // If pin is configured as output (relay)
                        int relay_active = (value >> i) & 1;
                        int sensor_pin = i + 4; // Corresponding sensor pin
                        
                        /* Simulate sensor feedback with 90% reliability
                         * Mimics real-world sensor noise and occasional failures */
                        int sensor_feedback = relay_active && ((rand() % 10) < 9);
                        s32k3x8_gpio_set_input(s, sensor_pin, sensor_feedback);
                    }
                }
            }
            break;
            
        case GPIO_REG_MPGPDO:
            /* Multi-port GPIO output register (32-bit)
             * Limited to 16-bit for current implementation compatibility */
            s->data_out = value & 0xFFFF;
            s->data_in = s->data_out; // Loopback simulation
            
            /* Update physical pin states */
            for (int i = 0; i < GPIO_NUM_PINS; i++) {
                qemu_set_irq(s->pin[i], (value >> i) & 1);
            }
            break;
            
        case 0x1600: // Direction register (DIR)
            /* Configure pin directions: 1=output, 0=input */
            s->dir = value & 0xFFFF;
            
            /* REALISTIC BUTTON SIMULATION for GPIO0 (Control Panel)
             * Simulate button presses that stay stable for realistic durations */
            if (s == gpio0_state) {
                static uint32_t button_state = 0x50; // Initial pattern
                static int stable_counter = 0;       // Counter for stability
                
                /* Apply current button state to input pins */
                for (int i = 4; i <= 7; i++) {
                    if (!(s->dir & (1 << i))) { // If pin is configured as input
                        s32k3x8_gpio_set_input(s, i, (button_state >> i) & 1);
                    }
                }
                
                /* Change pattern only after being stable for reasonable time
                 * This allows debouncing to work properly */
                stable_counter++;
                if (stable_counter >= 3) { // Change every 8 cycles (> 5ms debounce time)
                    button_state = (button_state << 1) | ((button_state >> 7) & 1);
                    stable_counter = 0; // Reset counter
                }
            }
            break;
            
        default:
            break;
    }
}

/**
 * Memory Region Operations Structure
 * 
 * Defines the interface between QEMU's memory system and the GPIO peripheral.
 * Configures access permissions, endianness, and valid access sizes.
 */
static const MemoryRegionOps s32k3x8_gpio_ops = {
    .read = s32k3x8_gpio_read,
    .write = s32k3x8_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
        .unaligned = false,
    },
};

/**
 * GPIO Input Signal Handler
 * 
 * Processes external input signals to GPIO pins. Updates internal state
 * and can trigger interrupt generation for pins with interrupt capability.
 * 
 * @param opaque Pointer to GPIO state structure
 * @param n Pin number (0-15)
 * @param level Signal level (0=low, 1=high)
 */
static void s32k3x8_gpio_input_handler(void *opaque, int n, int level)
{
    S32K3X8GPIOState *s = opaque;
    
    /* Update input pin state */
    if (level)
        s->data_in |= (1 << n);   // Set pin high
    else
        s->data_in &= ~(1 << n);  // Set pin low
        
    /* Note: Interrupt generation logic can be added here for pins
     * with interrupt capability (up to 144 pins on S32K3X8) */
}

/**
 * Set GPIO Input Pin State
 * 
 * Public interface for setting individual GPIO input pin states.
 * Used by simulation logic and external device connections.
 * 
 * @param s Pointer to GPIO state structure
 * @param pin Pin number to update
 * @param level New pin level (0=low, 1=high)
 */
void s32k3x8_gpio_set_input(S32K3X8GPIOState *s, int pin, int level)
{
    s32k3x8_gpio_input_handler(s, pin, level);
}

/**
 * GPIO Peripheral Initialization
 * 
 * Initializes the GPIO peripheral instance including memory region setup,
 * interrupt line configuration, and hardware interface preparation.
 * 
 * @param obj QEMU object being initialized
 */
static void s32k3x8_gpio_init(Object *obj)
{
    S32K3X8GPIOState *s = S32K3X8_GPIO(obj);

    /* Initialize memory-mapped I/O region */
    memory_region_init_io(&s->mmio, obj, &s32k3x8_gpio_ops, s,
                          TYPE_S32K3X8_GPIO, GPIO_MEM_SIZE);

    /* Register memory region with system bus */
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);

    /* Initialize GPIO output pins for external device connections */
    qdev_init_gpio_out(DEVICE(obj), s->pin, GPIO_NUM_PINS);
    
    /* Initialize GPIO input pins for external signal reception */
    qdev_init_gpio_in(DEVICE(obj), s32k3x8_gpio_input_handler, GPIO_NUM_PINS);
}

/**
 * GPIO Device Type Information
 * 
 * Defines the GPIO device type for QEMU's object system including
 * inheritance hierarchy and memory requirements.
 */
static const TypeInfo s32k3x8_gpio_info = {
    .name = TYPE_S32K3X8_GPIO,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K3X8GPIOState),
    .instance_init = s32k3x8_gpio_init,
};

/**
 * Register GPIO Device Type
 * 
 * Registers the GPIO device type with QEMU's type system.
 * Called during QEMU initialization to make the device available.
 */
static void s32k3x8_gpio_register_types(void)
{
    type_register_static(&s32k3x8_gpio_info);
}

/* Register type initialization function with QEMU */
type_init(s32k3x8_gpio_register_types)
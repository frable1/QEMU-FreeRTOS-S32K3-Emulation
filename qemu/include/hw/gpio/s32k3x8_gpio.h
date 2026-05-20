/*
 * S32K3X8 GPIO (General Purpose Input/Ouput)
 * Reference: NXP S32K3X8 Reference Manual
 */

#ifndef HW_S32K3X8_GPIO_H
#define HW_S32K3X8_GPIO_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_S32K3X8_GPIO "s32k3x8-gpio"
OBJECT_DECLARE_SIMPLE_TYPE(S32K3X8GPIOState, S32K3X8_GPIO)

/* pag. 20 reference manual
Up to 320 GPIO pins
Up to 144 GPIO pins with interrupt functionality
Up to 60 GPIO pins with wakeup capability
*/

#define NUM_GPIOS 8
#define GPIO_NUM_PINS 16

#define GPIO_DIR_OFFSET 0x1600
#define GPIO0_DIR (*(volatile uint32_t *)(GPIO0_BASE + GPIO_DIR_OFFSET))

void s32k3x8_gpio_set_input(S32K3X8GPIOState *s, int pin, int level);

struct S32K3X8GPIOState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;

    // only registers for S32K3X8
    uint32_t data_out;
    uint32_t data_in;
    uint32_t dir;
    uint32_t irq_enabled;
    uint32_t irq_status;
    qemu_irq pin[GPIO_NUM_PINS];
};

#endif

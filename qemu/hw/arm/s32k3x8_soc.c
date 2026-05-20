#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "hw/arm/boot.h"
#include "exec/address-spaces.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "sysemu/sysemu.h"
#include "hw/arm/s32k3x8_soc.h"
#include "hw/char/stm32l4x5_usart.h"
#include "hw/gpio/s32k3x8_gpio.h"

S32K3X8GPIOState *gpio0_state;
S32K3X8GPIOState *gpio1_state;

static void lpuarts_init(S32K3X8SOCState *s, DeviceState *armv7m, int n_lpuarts) {
    int uart_base_addr = UART_0_7_BASE_ADDR;
    int j = 0;
    
    for (int i = 0; i < n_lpuarts; i++, j++) {
        DeviceState *lpuart = qdev_new(TYPE_STM32L4X5_LPUART);
        qdev_prop_set_chr(lpuart, "chardev", serial_hd(i));

	    if (i == 0 || i == 1 || i == 8) {
            qdev_connect_clock_in(lpuart, "clk", s->aips_plat_clk);
        } else {
            qdev_connect_clock_in(lpuart, "clk", s->aips_slow_clk);
        }

        sysbus_realize_and_unref(SYS_BUS_DEVICE(lpuart), &error_fatal);

        // Compute base address for each LPUART
        if (i == 8) {
            j = 0;
            uart_base_addr = UART_8_15_BASE_ADDR;
        }
        
        // 0x4000 offset between LPUARTS
        hwaddr base_addr = uart_base_addr + (j * 0x4000);
        sysbus_mmio_map(SYS_BUS_DEVICE(lpuart), 0, base_addr);

        // Connect LPUART interrupt to CPU
        sysbus_connect_irq(SYS_BUS_DEVICE(lpuart), 0, qdev_get_gpio_in(armv7m, i));
    }
}

static void s32k3x8_soc_initfn(Object *obj) {
    S32K3X8SOCState *s = S32K3X8_SOC(obj);

    object_initialize_child(obj, "cpu", &s->cpu, TYPE_ARMV7M);

    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL, 0);
    s->refclk = qdev_init_clock_in(DEVICE(s), "refclk", NULL, NULL, 0);
    s->aips_plat_clk = qdev_init_clock_in(DEVICE(s), "aips_plat_clk", NULL, NULL, 0);
    s->aips_slow_clk = qdev_init_clock_in(DEVICE(s), "aips_slow_clk", NULL, NULL, 0);
}

static void s32k3x8_soc_realize(DeviceState *dev_soc, Error **errp) {
    S32K3X8SOCState *s = S32K3X8_SOC(dev_soc);
    MemoryRegion *system_memory = get_system_memory();
    DeviceState *armv7m;

    // FLASH memory configuration
    memory_region_init_rom(&s->flash_code, OBJECT(dev_soc), "S32K3X8.flash.code", FLASH_CODE_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, FLASH_CODE_BASE_ADDRESS, &s->flash_code);
    
    memory_region_init_rom(&s->flash_data, OBJECT(dev_soc), "S32K3X8.flash.data", FLASH_DATA_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, FLASH_DATA_BASE_ADDRESS, &s->flash_data);
    
    memory_region_init_rom(&s->utest, OBJECT(dev_soc), "S32K3X8.utest", UTEST_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, UTEST_BASE_ADDRESS, &s->utest);

    // RAM memory configuration
    memory_region_init_ram(&s->itcm0, NULL, "S32K3X8.itcm0", ITCM0_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, ITCM0_BASE_ADDRESS, &s->itcm0);
    
    memory_region_init_ram(&s->itcm2, NULL, "S32K3X8.itcm2", ITCM2_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, ITCM2_BASE_ADDRESS, &s->itcm2);
    
    memory_region_init_ram(&s->dtcm0, NULL, "S32K3X8.dtcm0", DTCM0_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, DTCM0_BASE_ADDRESS, &s->dtcm0);
    
    memory_region_init_ram(&s->dtcm2, NULL, "S32K3X8.dtcm2", DTCM2_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, DTCM2_BASE_ADDRESS, &s->dtcm2);
    
    memory_region_init_ram(&s->sram_stdby, NULL, "S32K3X8.sram_stdby", SRAM_STDBY_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, SRAM_STDBY_BASE_ADDRESS, &s->sram_stdby);
    
    memory_region_init_ram(&s->sram0, NULL, "S32K3X8.sram0", SRAM0_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, SRAM0_BASE_ADDRESS, &s->sram0);
    
    memory_region_init_ram(&s->sram1, NULL, "S32K3X8.sram1", SRAM1_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, SRAM1_BASE_ADDRESS, &s->sram1);
    
    memory_region_init_ram(&s->sram2, NULL, "S32K3X8.sram2", SRAM2_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, SRAM2_BASE_ADDRESS, &s->sram2);

    // ARM Cortex-M7 configuration
    armv7m = DEVICE(&s->cpu);
    qdev_prop_set_uint32(armv7m, "num-irq", 240);
    qdev_prop_set_uint8(armv7m, "num-prio-bits", 4);
    qdev_prop_set_string(armv7m, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m7"));
    qdev_prop_set_bit(armv7m, "enable-bitband", true);
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    qdev_connect_clock_in(armv7m, "refclk", s->refclk);
    object_property_set_link(OBJECT(&s->cpu), "memory", OBJECT(get_system_memory()), &error_abort);
    
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->cpu), errp)) {
        return;
    }
    
    // Initialization GPIO Port 0
    DeviceState *gpio0 = qdev_new(TYPE_S32K3X8_GPIO);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(gpio0), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(gpio0), 0, 0x40290000); // address: SIUL_VIRTWRAPPER_PDAC0 (memory_map.xlsx row 61)

    gpio0_state = S32K3X8_GPIO(gpio0);

    // Initialization GPIO Port 1
    DeviceState *gpio1 = qdev_new(TYPE_S32K3X8_GPIO);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(gpio1), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(gpio1), 0, 0x40298000); // SIUL_VIRTWRAPPER_PDAC1

    gpio1_state = S32K3X8_GPIO(gpio1);

    // Initialization GPIO Port 2
    DeviceState *gpio2 = qdev_new(TYPE_S32K3X8_GPIO);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(gpio2), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(gpio2), 0, 0x402A0000); // SIUL_VIRTWRAPPER_PDAC2

    //S32K3X8GPIOState *gpio2_state = S32K3X8_GPIO(gpio2);
    
    // LPUARTS configuration
    lpuarts_init(s, armv7m, 16);
}

static void s32k3x8_soc_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k3x8_soc_realize;
}

// SoC definition --> Associate the SoC type and initialization functions
static const TypeInfo s32k3x8_soc_info = {
    .name           = TYPE_S32K3X8_SOC,
    .parent         = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(S32K3X8SOCState),
    .instance_init  = s32k3x8_soc_initfn,
    .class_init     = s32k3x8_soc_class_init,
};

static void s32k3x8_soc_type(void) {
    type_register_static(&s32k3x8_soc_info);
}

type_init(s32k3x8_soc_type)


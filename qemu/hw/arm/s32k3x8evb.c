/*
 * NXP S32K3X8EVB Machine Model
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qemu/error-report.h"
#include "hw/arm/s32k3x8_soc.h"
#include "hw/arm/boot.h"

static void s32k3x8evb_init(MachineState *machine) {
    DeviceState *dev;
    Clock *sysclk;
    Clock *refclk;
    Clock *aips_plat_clk;
    Clock *aips_slow_clk;

    sysclk = clock_new(OBJECT(machine), "sysclk");
    clock_set_hz(sysclk, SYSCLK_FRQ);
    
    refclk = clock_new(OBJECT(machine), "refclk");
    clock_set_hz(refclk, REFCLK_FRQ);
    
    aips_plat_clk = clock_new(OBJECT(machine), "aips_plat_clk");
    clock_set_hz(aips_plat_clk, AIPS_PLAT_CLK_FRQ);

    aips_slow_clk = clock_new(OBJECT(machine), "aips_slow_clk");
    clock_set_hz(aips_slow_clk, AIPS_SLOW_CLK_FRQ);

    dev = qdev_new(TYPE_S32K3X8_SOC);
    object_property_add_child(OBJECT(machine), "soc", OBJECT(dev));
    
    qdev_connect_clock_in(dev, "sysclk", sysclk);
    qdev_connect_clock_in(dev, "refclk", refclk);
    qdev_connect_clock_in(dev, "aips_plat_clk", aips_plat_clk);
    qdev_connect_clock_in(dev, "aips_slow_clk", aips_slow_clk);
    
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    armv7m_load_kernel(ARM_CPU(first_cpu), machine->kernel_filename, 0, FLASH_CODE_SIZE);
}

static void s32k3x8evb_machine_init(MachineClass *mc) {
    static const char * const valid_cpu_types[] = {
        ARM_CPU_TYPE_NAME("cortex-m7"),
        NULL
    };

    mc->desc = "NXP S32K3X8EVB Board (Cortex-M7)";
    mc->init = s32k3x8evb_init;
    mc->valid_cpu_types = valid_cpu_types;
    mc->ignore_memory_transaction_failures = true;
}

DEFINE_MACHINE("s32k3x8evb", s32k3x8evb_machine_init)

#include "qemu/osdep.h"
#include "qemu/module.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "exec/memory.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include "hw/misc/unimp.h"
#include "stm32exp.h"

struct STM32EXPMcuClass {
    /*< private >*/
    SysBusDeviceClass parent_class;

    /*< public >*/
    const char *cpu_type;

    size_t flash_size;
};
typedef struct STM32EXPMcuClass STM32EXPMcuClass;
DECLARE_CLASS_CHECKERS(AVR32EXPMcuClass, AVR32EXP_MCU,
        TYPE_AVR32EXP_MCU)



// This functions sets up the device
static void stm32exp_realize(DeviceState *dev, Error **errp)
{
    //We create a state for the microcontroller form the generic state
    STTM32EXPMcuState *s = STM32EXP_MCU(dev);
    //And we create a class from the state
    const STM32EXPMcuClass *mc = STM32EXP_MCU_GET_CLASS(dev);

    // The AVR32 CPU was defined in the AVR32EXPMcuState
    object_initialize_child(OBJECT(dev), "cpu", &s->cpu, mc->cpu_type);
    //Set the CPU object to realized
    object_property_set_bool(OBJECT(&s->cpu), "realized", true, &error_abort);

    //Init the flash memory region
    memory_region_init_rom(&s->flash, OBJECT(dev),
                           "flash", mc->flash_size, &error_fatal);
    //Here we set the start address of the memory region 
    memory_region_add_subregion(get_system_memory(),
                                0x00400000, &s->flash);
}


//INDIRIZZI FLASH CAMBIATI
//
//

static void stm32exp_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    //Set the actuall setup function
    dc->realize = stm32exp_realize;
    dc->user_creatable = false;
}

static void stm32exps_class_init(ObjectClass *oc, void *data){

    STM32EXPMcuClass* stm32exp = STM32EXP_MCU_CLASS(oc);

    stm32exp->cpu_type = STM32A_CPU_TYPE_NAME("STM32EXPC");

    //FLASH SIZE CAMBIATA (NON SO SE E' GIUSTA LOL)
    stm32exp->flash_size = 12288 * KiB;
}

static const TypeInfo stm32exp_mcu_types[] = {
        {
                .name           = TYPE_STM32EXPS_MCU,
                .parent         = TYPE_STM32EXP_MCU,
                .class_init     = stm32exps_class_init,
        }, {
                .name           = TYPE_STM32EXP_MCU,
                .parent         = TYPE_SYS_BUS_DEVICE,
                .instance_size  = sizeof(STM32EXPMcuState),
                .class_size     = sizeof(STM32EXPMcuClass),
                .class_init     = stm32exp_class_init,
                .abstract       = true,
        }
};

DEFINE_TYPES(stm32exp_mcu_types)


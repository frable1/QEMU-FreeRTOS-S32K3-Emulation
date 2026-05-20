#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "stm32exp.h"
#include "boot.h"
#include "qom/object.h"
#include "hw/boards.h"
//h,mn,aSDFGHJSFDAdfghjgfdsaSD
#define TYPE_STM32_BOARD_BASE_MACHINE MACHINE_TYPE_NAME("stm32-board-base")
#define TYPE_STM32_BOARD_MACHINE MACHINE_TYPE_NAME("stm32-board")
DECLARE_OBJ_CHECKERS(STM32BoardMachineState, STM32BoardMachineClass,
        STM32_BOARD_MACHINE, TYPE_STM32_BOARD_MACHINE)

struct STM32BoardMachineState { 
    MachineState parent_obj;//we need to put every connected device here in the state
    STM32EXPMcuState mcu;
};
typedef struct STM32BoardMachineState STM32BoardMachineState;
 
struct STM32BoardMachineClass {
    MachineClass parent_class;//this contains attributes of the board. If we add an emulated memory region, we need to include the size here
};

//The generic MachineState is passed by QEMU
static void stm32_board_init(MachineState *machine)
{
    //Make a specific MachineState out of the generic one
    STM32BoardMachineState* m_state = STM32_BOARD_MACHINE(machine);

    //We initialize the mocrocontroller that is part of the board
    object_initialize_child(OBJECT(machine), "mcu", &m_state->mcu, TYPE_STM32EXPS_MCU);
    //And we connect it via QEMU's SYSBUS.
    sysbus_realize(SYS_BUS_DEVICE(&m_state->mcu), &error_abort);

    //Here we load the firmware file with a load function that we will implment in boot.c
    if (machine->firmware) {
        if (!stm32_load_firmware(&m_state->mcu.cpu, machine,
                                 &m_state->mcu.flash, machine->firmware)) {
            exit(1);
        }
    }
} 

//Generic Objectc is passed by QEMU
static void stm32_board_class_init(ObjectClass *oc, void *data)
{
    //The generic machine class from object
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "STM32 Board";
    mc->alias = "stm32-board";
    
    //Notice that we tell QEMU what function is used to initialize our board here.
    mc->init = stm32_board_init;
    mc->default_cpus = 1;
    mc->min_cpus = mc->default_cpus;
    mc->max_cpus = mc->default_cpus;
    // Our board does not have any media drive
    mc->no_floppy = 1;
    mc->no_cdrom = 1;
    //We also will not have threads
    mc->no_parallel = 1;
}

static const TypeInfo stm32_board_machine_types[] = {
        {
                                //Notice that this is the TYPE that we defined above.
                .name           = TYPE_STM32_BOARD_MACHINE,
                                //Our machine is a direct child of QEMU generic machine
                .parent         = TYPE_MACHINE,
                .instance_size  = sizeof(STM32BoardMachineState),
                .class_size     = sizeof(STM32BoardMachineClass),
                //We need to registers the class init function 
                .class_init     = stm32_board_class_init,
        }
};
DEFINE_TYPES(stm32_board_machine_types)
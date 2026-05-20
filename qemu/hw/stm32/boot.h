#ifndef HW_STM32_BOOT_H
#define HW_STM32_BOOT_H

#include "hw/boards.h"
#include "cpu.h"

bool stm32_load_firmware(STM32ACPU *cpu, MachineState *ms,
                         MemoryRegion *mr, const char *firmware);

#endif // HW_STM32_BOOT_H

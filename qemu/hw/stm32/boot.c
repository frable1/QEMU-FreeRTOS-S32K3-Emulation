#include "qemu/osdep.h"
#include "qemu/datadir.h"
#include "hw/loader.h"
#include "boot.h"
#include "qemu/error-report.h"

bool stm32_load_firmware(STM32ACPU *cpu, MachineState *ms,
                         MemoryRegion *program_mr, const char *firmware){
    g_autofree char *filename = NULL;
    int bytes_loaded;

    //We get the filename that is specified as 'bios' when QEMU is started
    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, firmware);
    if (filename == NULL) {
        error_report("Cannot find firmware image '%s'", firmware);
        return false;
    }
    //we use a build-in function to load the firmware from the file to the emulated memory.
    bytes_loaded = load_image_mr(filename, program_mr);

    if (bytes_loaded < 0) {
        error_report("Unable to load firmware image %s as ELF or raw binary",
                     firmware);
        return false;
    }
    return true;
}

#   NXP S32K3X8EVB emulated on QEMU and FreeRTOS

##  Introduction

This project is divided into two main phases, focusing on the emulation and development of embedded software for the NXP S32K3X8EVB evaluation board.

In the first part, we extend QEMU to emulate a board that is not officially supported: the NXP S32K3X8EVB. This involves customizing QEMU to support the correct ARM Cortex-M7 CPU, configuring the appropriate memory map, and implementing the peripherals assigned in the project spreadsheet (UART and GPIO). The goal is to create a faithful virtual environment that can be used for development and testing without the need for physical hardware.

The second part involves porting and running FreeRTOS on the emulated board. A port of FreeRTOS for the S32K3X8EVB already exists, and if the QEMU emulation is correctly implemented, the system should boot and execute FreeRTOS-based applications reliably.

To validate the success of both phases, we develop and run a simple application that uses multiple FreeRTOS tasks, demonstrating the correct functioning of both the emulation and the RTOS environment.

All technical information and documentation related to the board, including its memory map, peripheral configuration, and register descriptions, can be found in the `Manuals directory` of this project.

This guide outlines each step of the project, from the QEMU customization to the FreeRTOS integration and peripheral testing.

### Requirements

The project was developed on **Ubuntu 24.04 LTS** and the following packages are needed:

<!-- >   **check dependecies** -->

- **Core Build Tools**:
  - `build-essential` (includes `gcc`, `g++`, `make`)
  - `python3` (needed for QEMU build scripts)
  - `python3-pip` (for installing Python-related dependencies)

- **QEMU-Specific Build Dependencies**:
  - `git` (version control to clone repositories)
  - `libglib2.0-dev` (core library for QEMU)
  - `libfdt-dev` (for working with device tree files)
  - `libpixman-1-dev` (required for graphics emulation)
  - `zlib1g-dev` (for compression support)
  - `ninja-build` (for faster builds compared to Makefiles)
  -  `flex` and `bison` (required for QEMU's device emulation parsing logic)

- **ARM Toolchain**:
  - `gcc-arm-none-eabi` (the ARM GCC compiler)
  - `gdb-multiarch`  (a versatile debugger for cross-architecture debugging, including ARM)

To install all necessary dependencies run the following commands:
```sh
sudo apt update && sudo apt upgrade -y

sudo apt install -y \
    git build-essential python3 python3-pip \
    libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev \
    ninja-build flex bison \
    gcc-arm-none-eabi gdb-multiarch
```

### Project Structure

Create the main project directory, eventually the final project structure will be:
```
  project_name/
  ├── qemu/       
  ├── build/       
  ├── freertos/   
  └── demo/       
```

##  QEMU board emulation

### Qemu Setup
Clone the QEMU repository:
```sh
git clone https://github.com/qemu/qemu.git
cd qemu
```

### Creating the unsupported board NXP S32K3X8EVB
For the board implementation we used as a reference the already supported netduino2 board.

Based on this, we started by creating two files in the qemu/hw/arm directory:
  - [**s32k3x8_soc.c**](./qemu/hw/arm/s32k3x8_soc.c)
  - [**s32k3x8evb.c**](./qemu/hw/arm/s32k3x8evb.c)

and one header file in the qemu/include/hw/arm directory:
  - [**s32k3x8_soc.h**](./qemu/include/hw/arm/s32k3x8_soc.h)

#### s32k3x8_soc.c
This file defines the SoC implementation. 
  - `s32k3x8_soc_initfn`: 
      - Creates the CPU child object.
      - Initializes clock inputs.
  - `s32k3x8_soc_realize`:
      - Allocates and maps memory regions into the system's memory space.
      - Configures the Cortex-M7 CPU by setting properties and connecting clocks into it. Then it creates the CPU on the sysbus.
      - Creates and initializes GPIO devices.
      - Initializes LPUART peripherals by calling the `lpuarts_init(...)` function.
  - Lastly, the new device type is registered in QEMU.

#### s32k3x8_soc.h
In this header file we define
  - The structure for the SoC S32K3X8.

and the following values taken from the Reference Manual
  - Clock frequencies.
  - Memory regions' base addresses and sizes.
  - Uart peripherals' base addresses.


#### s32k3x8evb.c
This file defines the board model named s32k3x8evb.
-  `s32k3x8evb_init`:
    - Creates the clocks and sets their frequencies.
    - Creates the sock device and attaches it to the machine object.
    - Connects the clock into the SoC device.
    - Realizes the SoC on the system bus.
    - Loads the kernel into the flash.
- `s32k3x8evb_machine_init`:
    - Sets parameters like machine description, initialization function and allowed CPU types.
-  `DEFINE_MACHINE("s32k3x8evb", s32k3x8evb_machine_init)`:
    - Registers the machine name s32k3x8evb with QEMU.

### Testing the NXP S32K3X8EVB board
#### Build instructions

Create the *`/build`* folder in the root repository:

```sh
mkdir build
cd build
```

Launch QEMU configure:

```sh
../qemu/configure
```

Execute the Makefile (-j is used to speed up the execution):

```sh
make -j
```

Verify the presence of the *NXP S32K3X8EVB Board (Cortex-M7)* with:

```sh
./qemu-system-arm -M ?
```
```
...
realview-pbx-a9      ARM RealView Platform Baseboard Explore for Cortex-A9
romulus-bmc          OpenPOWER Romulus BMC (ARM1176)
s32k3x8evb           NXP S32K3X8EVB Board (Cortex-M7)
sabrelite            Freescale i.MX6 Quad SABRE Lite Board (Cortex-A9)
smdkc210             Samsung SMDKC210 board (Exynos4210)
...
```

#### Launch the board and visualize the memory tree
Still inside the *`/build`* folder, we lauch the board without the kernel with:

```sh
./qemu-system-arm -machine s32k3x8evb -nographic -S
```

To get the QEMU prompt we press: `Ctrl + 'a'` and then `'c'`.

Now, to see the memory tree we digit into the QEMU prompt:

```sh
info mtree
```
```
address-space: bitband-source
address-space: bitband-source
address-space: memory
  0000000000000000-ffffffffffffffff (prio -1, i/o): system
    0000000000000000-000000000000ffff (prio 0, ram): S32K3X8.itcm0
    0000000000010000-000000000001ffff (prio 0, ram): S32K3X8.itcm2
    0000000000400000-0000000000bfffff (prio 0, rom): S32K3X8.flash.code
    0000000010000000-000000001001ffff (prio 0, rom): S32K3X8.flash.data
    000000001b000000-000000001b001fff (prio 0, rom): S32K3X8.utest
    0000000020000000-000000002001ffff (prio 0, ram): S32K3X8.dtcm0
    0000000020400000-000000002040ffff (prio 0, ram): S32K3X8.sram_stdby
    0000000020410000-000000002043ffff (prio 0, ram): S32K3X8.sram0

...
```
#### Bare metal program testing

To validate that the custom board boots and executes code correctly, we added a minimal bare‑metal test in *`/qemu_test`*. It builds a tiny ELF and runs it on the emulated board so you can step through instructions with GDB.

What it does:
- Executes a small ARM loop that increments r0 from 0 to 10 and then returns.
- Useful to confirm: reset/entry works, vector table/stack are sane, code fetch/branches execute, and GDB stepping works.

Source ([qemu_test/main.c](./qemu_test/main.c)):
```c
int main(void) {
  asm("mov r0, #0 \n"
      "loop: \n"
        "add r0, r0, #1 \n"
        "cmp r0, #10 \n"
        "bne loop \n");
  
  return 0;
}
```

To run this test move to the *`/qemu_test`* folder and execute the Makefile with debug information

```sh
cd qemu_test
make debug
make qemu_debug   # Execute QEMU in debug mode
```

In another terminal, in the same *`/qemu_test`* folder, execute gdb:

```sh
make gdb_start
```

Now, in this GDB prompt execute:

```sh
target remote localhost:1234
layout regs
```

User GDB commands like `si` and `ni` to execute the programs instructions.


##  FreeRTOS porting


We use the existing FreeRTOS port for the S32K3X8EVB board as a baseline to validate our custom QEMU board implementation. Since this FreeRTOS port is already verified to work on the actual hardware, running it on our emulated board allows us to verify that the CPU, memory layout, and peripheral models (UART and GPIO) are emulated correctly and behave as expected.
If the application runs successfully in QEMU and produces the expected output, it indicates that our emulation accurately reflects the real hardware.

We include the upstream FreeRTOS repository as a Git submodule under *`/freertos`*. This keeps our repo small and lets us pin to a known-good version.

Add the submodule:
```sh
git submodule add https://github.com/FreeRTOS/FreeRTOS.git freertos
```

<!-- ### Application code
Our demo FreeRTOS application does the following:
  - Initializes the UART
  - Creates a single task that calls `test_gpio()` every 10 seconds
  - Starts the scheduler

The `test_gpio()` prints debug information over UART, so by looking at the output we can verify that the UART and GPIO peripherals have been correctly emulated. -->

### Linker Script
The linker script defines the memory layout for the emulated board, aligning it to the one described in the S32K3 Memory Guide (Pag. 15 of the S32K3MemoriesGuide.pdf that can be found in `Manuals` directory). It then maps different code and data sections to different memory regions. 

More specifically:
-  **ISR Vector table (`.isr_vector`)** is placed into the `ITCM0`, a memory with zero-wait access since an RTOS environment requires a short predictabe interrupt latency
-  **Executable code (`.text`), read-only data (`.rodata`) and constant data (`.const`)** are placed into the `PFLASH` non volatile memory
-  **Initialized data (`.data`), zero-initialized data (`.bss`) and heap (`.heap`)** are placed into the `DTCM`, a low latency RAM that is mapped directly onto the CPU bus and can be used for real-time data access
-  **Stack (`.stack`)** is placed into a separate `DTCM` in order to prevent accidental overwrites after a stack overflow
-  **Standby RAM (`.standby_ram`)** is placed into `SRAM_STDBY`, a memory region powered during low power or standby mode.
-  **Test data (`.utest`)** is placed into `UTEST`, a special flash memory used to keep debug info separated from other sections.

### Startup Code
The startup code is responsible for initializing the system after reset and handling exceptions or interrupts when running on bare metal. 

It includes: 
- `isr_vector[]`, an array that defines the interrupt vector table used by the CPU to determine where to jump after exceptions or interrupts. It includes the function addresses for the reset handler, fault handlers, and FreeRTOS kernel interrupt handlers. This table is placed in the `.isr_vector` section of memory and is mapped to ITCM0 by the linker script. When a reset happens, the CPU reads the initial stack pointer from the first word of the vector table, reads the address of the reset handler from the second word of the vector table and jumps to the reset handler
- `Reset_Handler()`. In our demo it simply calls `main()`
- `Default_Handler()`, used when an interrupt or exception without a defined handler occurs
- `HardFault_Handler()`, used when a HardFault is generated. The handler checks if the Main Stack Pointer or the Process Stack Pointer were active when the fault occurred, extracts the value of the stack pointer from the active stack and calls `prvGetRegistersFromStack()`

### Makefile
The Makefile automates compilation, linking and execution of the FreeRTOS application on the emulated board. 

It includes the following rules:
- ```make``` to build the ELF file
-  ```make qemu_start``` to run the ELF binary on the **s32k3x8evb** machine model of the custom QEMU build
- ```make qemu_debug``` to launch QEMU in paused mode
- ```make gdb_start``` to launch *gdb-multiarch* and load the ELF file
- ```make cleanobj``` to remove object files only
- ```make clean``` to remove ELF, linker map file and object files

### Build Instructions

Move to the *`/demo`* folder.

```sh
cd demo
```

Build the FreeRTOS application into an ELF binary.
```sh
make
```

Run the application. 
```sh
make qemu_start
```
More specifically, this rule: 
  - launches our custom QEMU build  
  - emulates the s32k3x8evb board  
  - loads the compiled FreeRTOS `demo.elf`  
  - redirects the UART output to the terminal



##  Peripherals

### UART

For our UART implementation, we reused one that belongs to the STM32 family, the STM32L4X5 USART. The QEMU files related to it are [`stm32l4x5_usart.h`](./qemu/include/hw/char/stm32l4x5_usart.h) and [`stm32l4x5_usart.c`](./qemu/hw/char/stm32l4x5_usart.c).

####  Hardware Reference & Integration Details

The following specifications are based on the **NXP S32K3X8 Reference Manual**:

- There are 16 LPUARTS (Low Power Universal Asynchronous Receiver-Transmitter).
- LPUARTS from 0 to 7 start from the address `0x40328000` and have an offset of `0x4000`.
- LPUARTS from 8 to 15 start from the address `0x4048C000` and have an offset of `0x4000`.
- LPUARTS 0, 1 and 8 use a clock frequency equals to `120 MHz` (AIPS_PLAT_CLK), while the others `60 MHz` (AIPS_SLOW_CLK).

#### QEMU Files and responsibilities

- [stm32l4x5_usart.c](./qemu/hw/char/stm32l4x5_usart.c)
  - Contains the implementation of the STM32L4X5 USART.

- [stm32l4x5_usart.h](./qemu/include/hw/char/stm32l4x5_usart.h)
  - It's the header file that we imported in s32k3x8_soc.c in order to instantiate the lpuarts devices.

- [s32k3x8_soc.h](./qemu/include/hw/arm/s32k3x8_soc.h)
  - Contains the definitions of the clock frequencies and the base addresses of the lpuarts.

- [s32k3x8evb.c](./qemu/hw/arm/s32k3x8evb.c)
  - In `s32k3x8evb_init`:
    - Creates the clocks:
      - `aips_plat_clk = clock_new(OBJECT(machine), "aips_plat_clk");`
    - Sets their frequencies:
      - `clock_set_hz(aips_plat_clk, AIPS_PLAT_CLK_FRQ);`
    - Connects them into the SoC device:
      - `qdev_connect_clock_in(dev, "aips_plat_clk", aips_plat_clk);`

- [s32k3x8_soc.c](./qemu/hw/arm/s32k3x8_soc.c)
  - In `lpuarts_init`:
    - Creates the lpuarts devices:
      - `DeviceState *lpuart = qdev_new(TYPE_STM32L4X5_LPUART);`
    - Configures them:
      - `qdev_prop_set_chr(lpuart, "chardev", serial_hd(i));`
      - `qdev_connect_clock_in(lpuart, "clk", s->aips_plat_clk);`
    - Connects them to the sysbus:
      - `sysbus_realize_and_unref(SYS_BUS_DEVICE(lpuart), &error_fatal);`
    - Computes for each lpuart its base address and maps the device in memory:
      - `sysbus_mmio_map(SYS_BUS_DEVICE(lpuart), 0, base_addr);`
    - Connects their interrupts to the CPU:
      - `sysbus_connect_irq(SYS_BUS_DEVICE(lpuart), 0, qdev_get_gpio_in(armv7m, i));`

#### UART Driver Implementation

In order to use the UART in the FreeRTOS application, we need to write a driver that initializes it and provides a function for printing output.

The driver implementation is based on STM's UART, as we used it to emulate our board. It defines several registers and the most important ones for us are:
  - `BRR` (Baud Rate): used to set the baud rate.
  - `CR1` (Control Register 1): used to enable the UART, receiver and transmitter (bits 0, 2 and 3 respectively).
  - `ISR` (Interrupt and Status Register): used to check if the transmission register is empty (bit 7).
  - `TDR` (Transmit Data Register): used to write the character to print in output.

Two files are needed: `uart.h` and `uart.c`.

- [uart.h](./demo/uart.h)
  - Defines the UART base address and the addresses of the registers:
    - `#define LPUART_BASE_ADDR 0x40328000`
    - `#define LPUART_CR1   (*(volatile uint32_t *)(LPUART_BASE_ADDR + 0x00))`
  - Defines a flag used in `uart.c` to check if the transmission register is empty or not:
    - `#define TXE_FLAG  (1 << 7)`

- [uart.c](./demo/uart.c)
  - In `UART_init`:
    - Configures the baud rate:
      - `LPUART_BRR = 16;`
    - Enables the transmitter and receiver:
      - `LPUART_CR1 = (1 << 3) | (1 << 2);`
    - Enables the UART:
      - `LPUART_CR1 |= (1 << 0);`
  - In `UART_printf`:
    - Waits for the transmission register to be empty:
      - `while (!(LPUART_ISR & TXE_FLAG)) {}`
    - Writes the character to TDR register:
      - `LPUART_TDR = (unsigned int)(*s);`

####  Uart Bare-Metal Testing


A minimal bare‑metal test in *`/uart_test`* initializes LPUART0 and prints a line over the emulated UART to verify MMIO mapping and basic UART behavior.

What it does:
- Programs the STM32L4X5‑based LPUART model at base `0x40328000`.
- Transmits `"Hello, World!"`.

Source ([uart_test/main.c](./uart_test/main.c)):
```c
#include <stdint.h>
#include "uart.h"

int main(void) {
  UART_init();
  
  char msg[] = "Hello, World!\n";
  UART_printf(msg);
  
  return 0;
}
```

To run this test move to the directory *`/uart_test`*, then execute the Makefile and run the program.

```sh
cd uart_test
make
make qemu_start
```



### GPIO

For our GPIO implementation, we began by referring to an existing GPIO reference header from the STM32 family: [`stm32l4x5_gpio.h`](./qemu/include/hw/gpio/stm32l4x5_gpio.h). While helpful as a template, the STM32 platform differs significantly from the S32K3X8 in how it handles GPIO functionality.

Unlike STM32, the NXP S32K3X8 microcontroller does not provide standalone GPIO modules. Instead, General-Purpose I/O is managed through the **SIUL2 (System Integration Unit Lite 2)** module, which handles both pin multiplexing and GPIO control on this platform.

####  Hardware Reference & Integration Details

The following specifications are based on the **NXP S32K3X8 Reference Manual** and the project's `memory_map.xlsx`:

- GPIO registers are accessible through the **SIUL_VIRTWRAPPER_PDACx** memory region.
- **Base address**: `0x40290000` (corresponding to PDAC0).
- Each wrapper region spans **16 KB**.

Within the emulation model, we defined several key registers with their offsets (from Reference Manual, p. 154):

```c
#define GPIO_REG_PGPDO   0x1700  // Parallel GPIO Data Output (16-bit)
#define GPIO_REG_PGPDI   0x1740  // Parallel GPIO Data Input (16-bit)
#define GPIO_REG_MPGPDO  0x1780  // Multi Parallel GPIO Data Output (32-bit)
#define GPIO_MEM_SIZE    0x4000  // Pin Direction Control (16-bit)
```

####  Practical Limitations & Pin Selection

Based on documentation (Reference Manual, p. 20) the S32K3X8 supports up to 320 GPIO pins, with:

- 144 pins featuring interrupt capability.
- 60 pins offering wake-up functionality.

To simplify the implementation and to focus on a subset of GPIO functionality for the demo application the following constants were chosen:

```c
#define NUM_GPIOS 8
#define GPIO_NUM_PINS 16
```

This allows us to manage up to 8 GPIO channels, each with up to 16 pins, sufficient for demonstrating basics like LED toggling, button input, and FreeRTOS task interaction.


#### Files and responsibilities

- [s32k3x8_gpio.h](./qemu/include/hw/gpio/s32k3x8_gpio.h)
  - Declares the device type: TYPE_S32K3X8_GPIO and S32K3X8GPIOState.
  - Defines basic limits (GPIO_NUM_PINS = 16) and any constants used by the device.
  - Exposes the helper API:
    - `void s32k3x8_gpio_set_input(S32K3X8GPIOState *s, int pin, int level)`
      - Sets an input level on a pin (used by other devices/tests).

- [s32k3x8_gpio.c](./qemu/hw/gpio/s32k3x8_gpio.c)
  - Core functions (base behavior only):
    - `s32k3x8_gpio_read(...)`
      - PGPDI: returns data_in masked to input pins (data_in & ~dir).
      - PGPDO: returns data_out masked to output pins (data_out & dir).
      - MPGPDO: returns data_out (for convenience).
    - `s32k3x8_gpio_input_handler(...)`
      - Updates data_in when an external signal drives a pin.
    - `s32k3x8_gpio_set_input(...)`
      - Thin wrapper calling the input handler (public helper).
    - `s32k3x8_gpio_init(...)`
      - Creates the MMIO region (memory_region_init_io with MemoryRegionOps).
      - Exposes GPIO lines: qdev_init_gpio_out/in for 16 pins.
      - Attaches MMIO to the sysbus device.
    - `s32k3x8_gpio_register_types(...)`
      - Registers the device type with QEMU (type_init).
  - Note: write-side testing/simulation logic is intentionally excluded here (kept for the demo use-case).

- [s32k3x8_soc.c](./qemu/hw/arm/s32k3x8_soc.c)
  - In realize/init:
    - Instantiates three GPIO devices (PDAC0–PDAC2) with `qdev_new(TYPE_S32K3X8_GPIO)`.
    - Realizes them on the sysbus and maps MMIO:
      - `sysbus_mmio_map(..., PDAC0_BASE)`
      - `sysbus_mmio_map(..., PDAC1_BASE)`
      - `sysbus_mmio_map(..., PDAC2_BASE)`
    - Optionally keeps pointers to gpio0/gpio1 for other SoC components (no testing logic here).

###  MPU (Memory Protection Unit)

The MPU has been integrated into the project to demonstrate memory protection capabilities in an embedded system running FreeRTOS. 

Its configuration and testing follow a two-step approach: initialization and self-test, followed by fault injection experiments.

#### Initialization

The MPU is configured in [`mpu.c`](./demo/mpu.c) and initialized in [`main.c`](./demo/main.c) right after the UART setup. Three regions have been defined:

- **Region 0 – Flash**
  - Base address: `0x00400000`
  - Size: 8 MB
  - Access: Read-Only

- **Region 1 – SRAM**
  - Base address: `0x20400000`
  - Size: 1 MB
  - Access: Read/Write

- **Region 2 – Peripheral space**
  - Base address: `0x40000000`
  - Size: 512 MB
  - Access: Read/Write

Additionally, a **NO-ACCESS subregion** has been defined inside SRAM (32 KB at `0x20408000`) to deliberately trigger faults during testing.

The MPU is enabled with the privileged default map so that privileged code can still access unmapped areas if needed.

#### Self-Test

At startup, the application runs `MPU_selftest()`, which:

1. Checks that the MPU is enabled and the privileged default map is active.

2. Verifies that each configured region (Flash, SRAM, Peripheral) matches the expected base address, size, and access permissions.

3. Performs a functional test by writing and reading back a word in SRAM.

4. Prints a summary (`MPU CONFIG: PASS` or `FAIL`) over UART.

This test confirms that the MPU registers have been correctly programmed.

If you do not want to see the MPU self-test output, simply comment out the call to `MPU_selftest();` in [`main.c`](./demo/main.c).

#### Fault Tests

Besides the automatic self-test, two additional tests have been implemented to demonstrate the MPU fault mechanism. They can be manually enabled in [`main.c`](./demo/main.c) by commenting/uncommenting function calls:

- `MPU_fault_test()`

  - Attempts to write into Flash, which is configured as Read-Only.
  - **Expected behavior**: a MemManage fault is raised.
  - Observed in QEMU: due to emulator limitations, the write may succeed silently, and the system may continue execution. On real hardware, this would correctly trap into the fault handler.

- `MPU_forced_fault()`

  - Attempts to write into the NO-ACCESS SRAM subregion.
  - **Expected and observed behavior**: a MemManage fault is raised, the program halts, and a diagnostic message is printed via UART.
  - This confirms that MPU protection works as intended under QEMU for SRAM.

#### Fault Handling

Both `MemManage_Handler` and `HardFault_Handler` are implemented in [`startup.c`](./demo/startup.c). When a violation occurs:

- The fault handler prints a diagnostic message over UART.

- Program execution stops, demonstrating that the MPU actively prevents unauthorized memory accesses.

## Demo application

To validate the emulated board and the peripherals we implemented, we wrote a simple FreeRTOS application that "exercises" the system in a way that resembles a small control panel. The aim is not to benchmark performance or cover every edge case, but to confirm that the basic contract between guest software and the QEMU device models (UART, GPIO) behaves as expected.

The app boots, initializes UART, and runs a periodic task that inspects inputs, updates outputs accordingly, and prints a human‑readable snapshot of the system state. This gives you immediate feedback that:
- MMIO mappings are correct,
- register semantics match the documentation,
- and the firmware can drive the virtual hardware end‑to‑end.

### Running

The main function ([demo/main.c](./demo/main.c)) initialize the UART and then execute a task that simply calls `test_gpio()` implemented in [demo/gpio.c](./demo/gpio.c), every 10 seconds. Each call performs one "control cycle": it reads inputs, applies a small amount of logic, drives outputs, then reports the result over UART. If you see clean, structured logs in your terminal, you know the emulator and the app are in sync.

Run it from the demo folder:
```sh
make
make qemu_start
```

### Application Behaviour

The GPIO demo models a basic control/actuation chain split across two ports:

- GPIO0 (the “control panel”)
  - Pins 4–7 are buttons (inputs).
  - Pins 0–3 are LEDs (outputs).
- GPIO1 (the “actuator side”)
  - Pins 0–3 are relays (outputs).
  - Pins 4–7 are sensors (inputs) that reflect relay activity.

Each cycle does this:
1. Configure directions once per port (outputs vs inputs).
2. Read the raw button bits from GPIO0.
3. Pass those bits through a software debouncer so we only react to stable presses.
4. Light up LEDs on GPIO0 to mirror the debounced buttons (button N → LED N).
5. Drive relays on GPIO1 with the same pattern (LED N → Relay N).
6. Read back sensors on GPIO1; in the emulator, they follow relays to emulate feedback.
7. Print a compact status line for buttons, LEDs, relays, sensors, plus a small debug line from the debouncer.

This forms a small loop: operator intent (buttons) → visual confirmation (LEDs) → actuation (relays) → feedback (sensors). Even in emulation, the pattern is useful to see that reads/writes and masking behave as they should.

### Debouncing 

Some boards may support hardware debouncing that is a mechanical switch: when pressed the physical contact can bounce, causing fluctuation in the input signal. This can cause unwanted reads on the GPIO pins. When this is not implemented, then software debouncing must be implemented.

Our demo treats the emulated inputs the same way and filters them. The debouncer waits until a new button state remains unchanged for a short time before accepting it. That way LEDs and relays don’t flicker on transient changes.

### Reading the output

The UART log shows four groups:
- Buttons: P = pressed, - = released (raw and debounced views)
- LEDs:    O = on,       - = off
- Relays:  A = active,   - = inactive
- Sensors: H = high,     L = low

An example output with both MPU fault tests disabled (only the self-test enabled) looks like:
```
--- MPU Self-Test ---
CTRL: EN=1 PRIVDEF=1
Region 0 (Flash RO 8MB):   PASS
Region 1 (SRAM RW 1MB):    PASS
Region 2 (PERIPH RW 512MB):PASS
SRAM RW test: PASS
MPU CONFIG: PASS
---------------------
Button debouncing initialized (2ms debounce time)

--- GPIO Industrial Control Test with Debouncing ---
Control Panel (GPIO0) - Debounced:
  Raw Buttons:       B0:P B1:- B2:P B3:-
  Debounced Buttons: B0:- B1:- B2:- B3:-
  LEDs:              L0:- L1:- L2:- L3:-
Actuators (GPIO1):
  Relays:  R0:- R1:- R2:- R3:-
  Sensors: S0:L S1:L S2:L S3:L
[Debug] Tick: 1, Stable: 0x0
-----------------------

--- GPIO Industrial Control Test with Debouncing ---
Control Panel (GPIO0) - Debounced:
  Raw Buttons:       B0:P B1:- B2:P B3:-
  Debounced Buttons: B0:- B1:- B2:- B3:-
  LEDs:              L0:- L1:- L2:- L3:-
Actuators (GPIO1):
  Relays:  R0:- R1:- R2:- R3:-
  Sensors: S0:L S1:L S2:L S3:L
[Debug] Tick: 2, Stable: 0x0
-----------------------

--- GPIO Industrial Control Test with Debouncing ---
[DEBOUNCE] State stabilized: 0x5
Control Panel (GPIO0) - Debounced:
  Raw Buttons:       B0:P B1:- B2:P B3:-
  Debounced Buttons: B0:P B1:- B2:P B3:-
  LEDs:              L0:O L1:- L2:O L3:-
Actuators (GPIO1):
  Relays:  R0:A R1:- R2:A R3:-
  Sensors: S0:H S1:L S2:H S3:L
[Debug] Tick: 3, Stable: 0x5
-----------------------

--- GPIO Industrial Control Test with Debouncing ---
Control Panel (GPIO0) - Debounced:
  Raw Buttons:       B0:- B1:P B2:- B3:P
  Debounced Buttons: B0:P B1:- B2:P B3:-
  LEDs:              L0:O L1:- L2:O L3:-
Actuators (GPIO1):
  Relays:  R0:A R1:- R2:A R3:-
  Sensors: S0:H S1:L S2:H S3:L
[Debug] Tick: 4, Stable: 0x5
-----------------------

--- GPIO Industrial Control Test with Debouncing ---
Control Panel (GPIO0) - Debounced:
  Raw Buttons:       B0:- B1:P B2:- B3:P
  Debounced Buttons: B0:P B1:- B2:P B3:-
  LEDs:              L0:O L1:- L2:O L3:-
Actuators (GPIO1):
  Relays:  R0:A R1:- R2:A R3:-
  Sensors: S0:H S1:L S2:H S3:L
[Debug] Tick: 5, Stable: 0x5
-----------------------

--- GPIO Industrial Control Test with Debouncing ---
[DEBOUNCE] State stabilized: 0xA
Control Panel (GPIO0) - Debounced:
  Raw Buttons:       B0:- B1:P B2:- B3:P
  Debounced Buttons: B0:- B1:P B2:- B3:P
  LEDs:              L0:- L1:O L2:- L3:O
Actuators (GPIO1):
  Relays:  R0:- R1:A R2:- R3:A
  Sensors: S0:L S1:H S2:L S3:H
[Debug] Tick: 6, Stable: 0xA
-----------------------
...
```


##  Conclusion

This project successfully integrates **FreeRTOS** with a custom board emulated in **QEMU** , extending its functionality with  **UART** , **GPIO debouncing** , and **MPU support**.

- The **UART module** enables formatted text output, which is essential for debugging and for visualizing the results of all system tests.

- The **GPIO module** implements button debouncing and actuator/sensor simulation, showing stable and realistic I/O behavior under FreeRTOS scheduling.

- The **MPU self-test** verifies the correct configuration of memory regions (Flash, SRAM, Peripheral).

- The **MPU fault tests** demonstrate how illegal memory accesses are trapped and handled, confirming the effectiveness of the protection mechanism.

In summary:

- FreeRTOS tasks run cooperatively with UART and GPIO peripherals.

- The MPU provides additional robustness by enforcing memory access permissions.

- All functionalities can be reproduced and validated through QEMU, following the provided instructions.

This proves that the integration of **FreeRTOS** , **UART communication** , **GPIO management** , and **MPU protection** is functional and reliable. The project serves as a solid foundation for more complex embedded applications that require real-time task scheduling, safe memory protection, and dependable I/O handling.

## License

This project is licensed under the [Creative Commons Attribution 4.0 International License](./LICENSE.md).

#include "FreeRTOS.h"
#include "task.h"

#include "uart.h"
#include "gpio.h"
#include "mpu.h"

#define mainTASK_PRIORITY    ( tskIDLE_PRIORITY + 2 )

void vTaskFunction(void *pvParameters);
extern void MPU_selftest(void);

int main(int argc, char **argv){

	(void) argc;
	(void) argv;

	UART_init();
	MPU_init();
	MPU_selftest();
	// OPTIONAL: enable this if you want to trigger a fault
	//MPU_fault_test();
	MPU_forced_fault();

	xTaskCreate(
		// Function which implements the task
		vTaskFunction,
		// Name of the task (debug purposes, not used by the kernel)
		"Task1",
		// Stack to allocate to the task
		configMINIMAL_STACK_SIZE,
		// Parameter passed to the task. Not needed for Hello World example
		NULL,
		// Priority assigned to the task
		mainTASK_PRIORITY,
		// Task handle. Not required
		NULL
	);

	// Give control to the scheduler
	vTaskStartScheduler();

	// If everything ok should never reach here
    for( ; ; );
}

/* Task Function */
void vTaskFunction(void *pvParameters) {

	// Avoid warning about unused pvParameters
	(void) pvParameters;

    for (;;) {

        // Task code: print a message
        // UART_printf("Hello, World!\n");
		test_gpio();

        // Delay for 10 seconds
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

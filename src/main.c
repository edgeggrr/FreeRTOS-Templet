#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

// Task function to print the core it is currently running on
void vTestTask(void *pvParameters) {
    const char *task_name = (const char *)pvParameters;
    
    while (1) {
        // Retrieve the current core executing this task (0 or 1)
        uint core_num = get_core_num();
        
        printf("%s is running on Core %u\n", task_name, core_num);
        
        // Delay for 1000 milliseconds to prevent spamming the UART
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main() {
    // Initialize standard I/O for UART/USB serial output
    stdio_init_all();
    
    // Initialize the CYW43 architecture (Mandatory for Pico W / Pico 2 W LED control)
    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43 architecture\n");
        return -1;
    }

    printf("Setting up FreeRTOS Tasks...\n");

    TaskHandle_t xTask1Handle = NULL;
    TaskHandle_t xTask2Handle = NULL;

    // Create two identical tasks
    xTaskCreate(vTestTask, "Task A", 256, (void *)"Task A", 1, &xTask1Handle);
    xTaskCreate(vTestTask, "Task B", 256, (void *)"Task B", 1, &xTask2Handle);

    /* 
     * OPTIONAL: Force tasks to specific cores to explicitly prove SMP works.
     * Uncomment these lines if the scheduler happens to run both on Core 0.
     */
    // vTaskCoreAffinitySet(xTask1Handle, (1 << 0)); // Pin Task A to Core 0
    // vTaskCoreAffinitySet(xTask2Handle, (1 << 1)); // Pin Task B to Core 1

    printf("Starting FreeRTOS Scheduler...\n");
    
    // Hand over control to FreeRTOS. This function should never return.
    vTaskStartScheduler();

    // =========================================================================
    // FAILURE STATE: The code below ONLY runs if the scheduler fails to start.
    // =========================================================================
    
    // Turn ON the Pico 2 W onboard LED to signal a failure
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    
    while (1) {
        // Trap the processor in an infinite loop
        tight_loop_contents();
    }
    
    return 0;
}
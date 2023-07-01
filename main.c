/**
 * PicoPlayOpus
 * A simple Opus player for Raspberry Pi Pico.
 * This is mostly intended as a starting point for other projects.
 */

#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"
#include "settings.h"

static TaskHandle_t appTaskHandle;
static void App_Task(void * argument);

static TaskHandle_t usbTaskHandle;
static void USB_Task(void * argument);

void App_Init(void) {
#if CLOCK_SPEED_KHZ != 133000
    set_sys_clock_khz(CLOCK_SPEED_KHZ, true);
#endif

    xTaskCreate( App_Task,             /* The function that implements the task. */
                 "App",                /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                 APP_TASK_STACK_SIZE,  /* The size of the stack to allocate to the task. */
                 NULL,                 /* The parameter passed to the task - not used in this case. */
                 APP_TASK_PRIORITY,    /* The priority assigned to the task. */
                 &appTaskHandle );     /* The task handle is not required, so NULL is passed. */

    xTaskCreate( USB_Task,
                 "USB",
                 USB_TASK_STACK_SIZE,
                 NULL,
                 USB_TASK_PRIORITY,
                 &usbTaskHandle );
}

static void App_Task(void * argument) {
    (void) argument;  // Unused parameter

    while (1) {
        uint32_t lastCore = get_core_num();
        printf("Hello from core %d!\n", lastCore);
        vTaskDelay(1000);
    }   
}

static void USB_Task(void * argument) {
    (void) argument;  // Unused parameter

    while (1) {
        tud_task(); // device task
    }   
}

int main() {
    App_Init();

    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was insufficient FreeRTOS heap memory available for the Idle and/or
    timer tasks to be created.  See the memory management section on the
    FreeRTOS web site for more details on the FreeRTOS heap
    http://www.freertos.org/a00111.html. */
    for( ;; );
}

void vApplicationMallocFailedHook( void ) {
    /* The malloc failed hook is enabled by setting
    configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.
    Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    panic("malloc failed");
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
    ( void ) pcTaskName;
    ( void ) xTask;

    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  pxCurrentTCB can be
    inspected in the debugger if the task name passed into this function is
    corrupt. */
    for( ;; );
}

/** @} */

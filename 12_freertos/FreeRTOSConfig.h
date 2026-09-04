#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configCPU_CLOCK_HZ                    (16000000UL)
#define configTICK_RATE_HZ                   (1000)
#define configMAX_PRIORITIES                  5
#define configMINIMAL_STACK_SIZE             128
#define configTOTAL_HEAP_SIZE                (16 * 1024)
#define configMAX_TASK_NAME_LEN               16

#define configUSE_PREEMPTION                  1
#define configUSE_IDLE_HOOK                   0
#define configUSE_TICK_HOOK                   0
#define configUSE_16_BIT_TICKS                0

#define configSUPPORT_DYNAMIC_ALLOCATION      1
#define configSUPPORT_STATIC_ALLOCATION       0

#define configUSE_MUTEXES                     1
#define configUSE_COUNTING_SEMAPHORES         1

#define configPRIO_BITS                       4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configSYSTICK_CLOCK_HZ                (configCPU_CLOCK_HZ)

#define INCLUDE_vTaskDelay                   1

#define configASSERT(x) \
    if (!(x)) { taskDISABLE_INTERRUPTS(); for (;;) {} }

#endif

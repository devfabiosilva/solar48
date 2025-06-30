#include <FreeRTOS/FreeRTOSConfig.h>

.syntax unified
.cpu cortex-m3
.thumb

.global xPortPendSVHandler
.global vPortSVCHandler
.global vPortStartFirstTask
.extern pxCurrentTCB
.extern vTaskSwitchContext

xPortPendSVHandler:
    mrs r0, psp
    isb
    ldr r3, =pxCurrentTCB
    ldr r2, [r3]

    stmdb r0!, {r4-r11}       // Save registers r4-r11 to task stack
    str r0, [r2]

    stmdb sp!, {r3, lr}
    mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
    msr basepri, r0
    dsb
    isb
    bl vTaskSwitchContext
    mov r0, #0
    msr basepri, r0
    ldmia sp!, {r3, lr}

    ldr r1, [r3]
    ldr r0, [r1]
    ldmia r0!, {r4-r11}
    msr psp, r0
    isb
    bx lr

vPortSVCHandler:
    ldr r3, =pxCurrentTCB
    ldr r1, [r3]
    ldr r0, [r1]
    ldmia r0!, {r4-r11}
    msr psp, r0
    isb
    mov r0, #0
    msr basepri, r0
    bx lr

vPortStartFirstTask:
    ldr r0, =0xE000ED08
    ldr r0, [r0]
    ldr r0, [r0]
    msr msp, r0
    cpsie i
    cpsie f
    dsb
    isb
    svc 0
    bx lr


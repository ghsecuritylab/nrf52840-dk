/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example_freertos
 *
 * @brief Blinky FreeRTOS Example Application main file.
 *
 * This file contains the source code for a sample application using FreeRTOS to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

#define TASK_DELAY       1000           /**< Task delay. Delays a LED0 task for 200 ms */

/* Declare a variable that is used to hold the handle of Task 2. */
TaskHandle_t xTask2Handle = NULL;

/**@brief task 1 entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void vTask1 (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    const char *pcTaskName = "Task 1 is running\r\n";
    UBaseType_t uxPriority;

    /* This task will always run before Task 2 as it is created with the higher
       priority. Neither Task 1 nor Task 2 ever block so both will always be in
       either the Running or the Ready state.

       Query the priority at which this task is running - passing in NULL means
       "return the calling task's priority".
    */
    uxPriority = uxTaskPriorityGet( NULL );

    /* As per most tasks, this task is implemented in an infinite loop.*/
    while (true)
    {
        NRF_LOG_INFO(pcTaskName);

        /* Setting the Task 2 priority above the Task 1 priority will cause
           Task 2 to immediately start running ( as then Task 2 will have the
           higher priority of the two created tasks). Note the use of the handle
           to task 2 ( xTask2Handle) in the call to vTaskPrioritySet().
        */
        NRF_LOG_INFO("About to raise the Task 2 priority");

        NRF_LOG_FLUSH();

        vTaskPrioritySet( xTask2Handle, (uxPriority + 1));
        /* Task 1 will only run when it has a priority higher than Task 2.
           Therefore, for this task to reach this point, Task 2 must already have
           executed and set its priority back down to below the priority of this
           task.
        */
    }
}

/**@brief task 2 entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void vTask2 (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    const char *pcTaskName = "Task 2 is running\r\n";
    UBaseType_t uxPriority;

    /* Task 1 will always run before this task as Task 1 is created with a higher
       priority. Neither Task 1 nor Task 2 ever block so will always be in either
       the Running or the Ready state.

       Query the priority at which this task is running - passing in NULL means
       "return the calling task's priority."
    */
    uxPriority = uxTaskPriorityGet( NULL );

    /* As per most tasks, this task is implemented in an infinite loop.*/
    while (true)
    {
        NRF_LOG_INFO(pcTaskName);

        NRF_LOG_INFO("About to lower the Task 2 priority");

        NRF_LOG_FLUSH();

        vTaskPrioritySet(NULL, (uxPriority - 2));

        /* Tasks must be implemented to never return... */
    }
}

int main(void)
{
    ret_code_t err_code;

    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    /* Configure LED-pins as outputs */
    bsp_board_init(BSP_INIT_LEDS);

    /* Initialize NRF LOG */
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("FreeRTOS Task Example\r\n");
    /* Create task for LED0 blinking with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(vTask1, "vTask1", configMINIMAL_STACK_SIZE + 200, NULL, 2, NULL));
    UNUSED_VARIABLE(xTaskCreate(vTask2, "vTask2", configMINIMAL_STACK_SIZE + 200, NULL, 1, &xTask2Handle));
    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    while (true)
    {
        /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
         * in vTaskStartScheduler function. */
    }
}

/**
 *@}
 **/

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
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_error.h"
#include "nrf_ringbuf.h"

#include "gps_buffer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

#define RING_BUFFER_LEN           8
uint8_t gps_lat_s[LAT_DATA_SIZE] = "10.212";
uint8_t gps_lon_s[LON_DATA_SIZE] = "30.212";
uint8_t gps_ts_s[TIME_DATA_SIZE] = "14072019";

uint8_t gps_lat_d[LAT_DATA_SIZE] = "11.221";
uint8_t gps_lon_d[LON_DATA_SIZE] = "45.212";
uint8_t gps_ts_d[TIME_DATA_SIZE] = "15072019";

void print_buffer_status(cbuf_handle_t cbuf);

/**@brief task 1 entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void vTask1 (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    const char *pcTaskName = "Task 1 is running\r\n";
    const TickType_t xDelay1000ms = pdMS_TO_TICKS( 1000UL );
    gps_buf_t gps_buf_i;
    //gps_buf_t gps_buf_o;
    gps_buf_t * buffer  = circular_buf_allocate_mem(RING_BUFFER_LEN);
    NRF_LOG_INFO("\n=== C Circular Buffer Check ===\n");
    cbuf_handle_t cbuf = circular_buf_init(buffer, RING_BUFFER_LEN);
    NRF_LOG_INFO("Buffer initialized. \r\n");

    /* As per most tasks, this task is implemented in an infinite loop.*/
    while (true)
    {
        NRF_LOG_INFO(pcTaskName);
        NRF_LOG_FLUSH();
        snprintf((char*)gps_buf_i.lat, LAT_DATA_SIZE, "%s", gps_lat_s);
        snprintf((char*)gps_buf_i.lon, LON_DATA_SIZE, "%s", gps_lon_s);
        snprintf((char*)gps_buf_i.time_stamp, TIME_DATA_SIZE, "%s", gps_ts_s);
        circular_buf_put(cbuf, gps_buf_i);
        print_buffer_status(cbuf);
        vTaskDelay(xDelay1000ms);
    }
    free(buffer);
    circular_buf_free(cbuf);
}


int main(void)
{
    ret_code_t err_code;

    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    /* Initialize NRF LOG */
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("FreeRTOS Task Example\r\n");
    /* Create task for LED0 blinking with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(vTask1, "vTask1", configMINIMAL_STACK_SIZE + 200, NULL, 1, NULL));
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

 void print_buffer_status(cbuf_handle_t cbuf)
 {
 	NRF_LOG_INFO("Full: %d, empty: %d, size: %u\n",
 		circular_buf_full(cbuf),
 		circular_buf_empty(cbuf),
 		circular_buf_size(cbuf));
 }
/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#include "tm1637.h"
#include <time.h>
#include <sys/time.h>

static const char *TAG = "example";

#define BLINK_GPIO 8

static uint8_t s_led_state = 0;
static uint16_t s_led_period = 1000;

static uint16_t s_wait_time = 1000 / portTICK_PERIOD_MS;
const gpio_num_t LED_CLK = CONFIG_TM1637_CLK_PIN;
const gpio_num_t LED_DTA = CONFIG_TM1637_DIO_PIN;

void tm1637_task(void *arg)
{
    tm1637_led_t *led = tm1637_init(LED_CLK, LED_DTA);
    if (led == NULL)
        vTaskDelete(NULL);

    #if 0
        tm1637_set_brightness(led, 7);
        while (true)
        {
            tm1637_set_segment_fixed(led, led->segment_idx[0], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[1], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[2], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[3], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[4], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[5], 0xFF);
            vTaskDelay(s_wait_time);
        }
    #endif

    while (true)
    {
        // Test segment control
        ESP_LOGI(TAG, "Test segment control");
        uint8_t seg_data[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
        for (uint8_t x = 0; x < 32; ++x)
        {
            uint8_t v_seg_data = seg_data[x % 6];
            tm1637_set_segment_fixed(led, led->segment_idx[0], v_seg_data);
            tm1637_set_segment_fixed(led, led->segment_idx[1], v_seg_data);
            tm1637_set_segment_fixed(led, led->segment_idx[2], v_seg_data);
            tm1637_set_segment_fixed(led, led->segment_idx[3], v_seg_data);
            tm1637_set_segment_fixed(led, led->segment_idx[4], v_seg_data);
            tm1637_set_segment_fixed(led, led->segment_idx[5], v_seg_data);
            vTaskDelay(s_wait_time);
        }

        // Test brightness
        ESP_LOGI(TAG, "Test brightness");
        for (int x = 0; x < 1; x++)
        {
            tm1637_set_brightness(led, x);
            tm1637_set_segment_fixed(led, led->segment_idx[0], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[1], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[2], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[3], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[4], 0xFF);
            tm1637_set_segment_fixed(led, led->segment_idx[5], 0xFF);
            vTaskDelay(s_wait_time);
        }
        vTaskDelay(s_wait_time);

        // Test display integer number
        ESP_LOGI(TAG, "Test display integer number");
        tm1637_set_number(led, 1, true, 0x00); // 0001
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, 12, true, 0x00); // 0012
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, 123, true, 0x00); // 0123
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, 1234, true, 0x00); // 1234
        vTaskDelay(s_wait_time);

        tm1637_set_number(led, 1, false, 0x00); // ____1
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, 12, false, 0x00); // ____12
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, 123, false, 0x00); // ___123
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, 1234, false, 0x00); // __1234
        vTaskDelay(s_wait_time);

        tm1637_set_number(led, -1, true, 0x00); // -001
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, -12, true, 0x00); // -012
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, -123, true, 0x00); // -123
        vTaskDelay(s_wait_time);

        tm1637_set_number(led, -1, false, 0x00); // ____-1
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, -12, false, 0x00); // ___-12
        vTaskDelay(s_wait_time);
        tm1637_set_number(led, -123, false, 0x00); // __-123
        vTaskDelay(s_wait_time);

        // Test display text
        ESP_LOGI(TAG, "Test display text");
		tm1637_set_segment_ascii(led, "PLAY");
		vTaskDelay(s_wait_time);
		tm1637_set_segment_ascii(led, "1234567890");
		vTaskDelay(s_wait_time);
		tm1637_set_segment_ascii(led, "IP 192.168.10.20");
		vTaskDelay(s_wait_time);
		tm1637_set_segment_ascii(led, "STOP");
		vTaskDelay(s_wait_time);

        // Test clock segment
        ESP_LOGI(TAG, "Test clock segment 1");
        tm1637_set_segment_ascii_with_time(led, "1234", 0x40, 1000);
        vTaskDelay(s_wait_time);
        ESP_LOGI(TAG, "Test clock segment 2");
        tm1637_set_segment_ascii_with_time(led, "1234", 0x00, 1000);
        vTaskDelay(s_wait_time);
    } // end while
}

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

void led_task(void *arg)
{
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (1)
    {
        // ESP_LOGI(TAG, "s_led_period %d", s_led_period);
        // ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(s_led_period / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{

    ESP_LOGI(TAG, "GPS LOG");

    xTaskCreate(&tm1637_task, "tm1637_task", 1024 * 4, NULL, 5, NULL);
    xTaskCreate(&led_task, "led_task", 1024 * 2, NULL, 5, NULL);
}

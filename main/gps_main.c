/* ESP Gps

   Receive NMEA signals from a GPS and show speed in a display

   Future development:
        Add road radar database and monitor distance to radars
        Signal radar presence.
        Show average speed for section radars
        Add a buzzer to signal when speed is over the limit
        Add a RGB led to signal states
        Add a way to update the radar database
*/
#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "nmea_parser.h"
#include "tm1637.h"
#include <time.h>
#include <sys/time.h>
#include <math.h>

static const char *TAG = "EspGps";

#define BLINK_GPIO 8
#define TIME_ZONE (+1)   // Europe Time
#define YEAR_BASE (2000) // date in GPS starts from 2000

static uint8_t s_led_state = 0;
static uint16_t s_led_period = 1000;
static uint16_t s_led_period_fast = 250;
// static uint16_t s_wait_time = 500 / portTICK_PERIOD_MS;
const gpio_num_t LED_CLK = CONFIG_TM1637_CLK_PIN;
const gpio_num_t LED_DTA = CONFIG_TM1637_DIO_PIN;
tm1637_led_t *led = NULL;

// uint16_t speed_kmh = 0;
uint8_t fix_state = GPS_MODE_INVALID;  // GPS_MODE_INVALID, GPS_MODE_2D, GPS_MODE_3D
static const int s_led_brightness = 7; // 0-7

// void tm1637_task(void *arg)
void show_speed(uint16_t speed_kmh)
{
    // tm1637_led_t *led = tm1637_init(LED_CLK, LED_DTA);
    // if (led == NULL)
    //     vTaskDelete(NULL);

    // tm1637_set_brightness(led, s_led_brightness);

    // while (true)
    // {
    ESP_LOGI(TAG, "Display speed %i", speed_kmh);
    tm1637_set_number(led, speed_kmh, false, 0x00);
    //     vTaskDelay(s_wait_time);
    // }
}

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    gps_t *gps = NULL;
    switch (event_id)
    {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        ESP_LOGI(TAG, "%d/%d/%d %d:%d:%d => \r\n"
                      "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
                      "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
                      "\t\t\t\t\t\taltitude   = %.02fm\r\n"
                      "\t\t\t\t\t\tspeed      = %fm/s",
                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
                 gps->latitude, gps->longitude, gps->altitude, gps->speed);

        // taskENTER_CRITICAL();
        uint16_t speed_kmh_calc = round(gps->speed * 3.6); // convert m/s to km/h
        show_speed(round(gps->speed));
        // speed_kmh = speed_kmh_calc;
        fix_state = gps->fix_mode;
        // taskEXIT_CRITICAL();
        break;
    case GPS_UNKNOWN:
        /* print unknown statements */
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        show_speed(0);
        break;
    default:
        show_speed(0);
        break;
    }
}

void led_task(void *arg)
{
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (true)
    {
        switch (fix_state)
        {
        case GPS_MODE_INVALID:
            gpio_set_level(BLINK_GPIO, s_led_state);
            s_led_state = !s_led_state;
            vTaskDelay(s_led_period / portTICK_PERIOD_MS);
            break;
        case GPS_MODE_2D:
            gpio_set_level(BLINK_GPIO, 1);
            s_led_state = !s_led_state;
            vTaskDelay(s_led_period_fast / portTICK_PERIOD_MS);
            break;
        case GPS_MODE_3D:
            gpio_set_level(BLINK_GPIO, 0);
            vTaskDelay(s_led_period / portTICK_PERIOD_MS);
            break;
        default:
            break;
        }
    }
    // if (fix_state == GPS_MODE_INVALID)
    // {
    //     gpio_set_level(BLINK_GPIO, s_led_state);
    //     s_led_state = !s_led_state;
    // }
    // else
    // {
    //     gpio_set_level(BLINK_GPIO, 1);
    // }
    // vTaskDelay(s_led_period / portTICK_PERIOD_MS);
}

void app_main(void)
{

    ESP_LOGI(TAG, "GPS LOG");

    led = tm1637_init(LED_CLK, LED_DTA);
    tm1637_set_brightness(led, s_led_brightness);
    // xTaskCreate(&tm1637_task, "tm1637_task", 1024 * 4, NULL, 5, NULL);

    xTaskCreate(&led_task, "led_task", 1024 * 2, NULL, 5, NULL);

    /* NMEA parser configuration */
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
    /* init NMEA parser library */
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&config);
    /* register event handler for NMEA parser library */
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
}

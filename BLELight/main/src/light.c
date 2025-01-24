#include "light.h"

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "light";

#define BLUE_GPIO 18
#define WHITE_GPIO 19

static uint8_t blue_state = 0;
static uint8_t white_state = 0;

void light_init() {
    gpio_reset_pin(BLUE_GPIO);
    gpio_set_direction(BLUE_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(WHITE_GPIO);
    gpio_set_direction(WHITE_GPIO, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Init lights");
}

void blue_toggle() {
    blue_state = !blue_state;
    gpio_set_level(BLUE_GPIO, blue_state);
    ESP_LOGI(TAG, "Turning the blue LED %s!", blue_state == true ? "ON" : "OFF");
}

void white_toggle() {
    white_state = !blue_state;
    gpio_set_level(WHITE_GPIO, white_state);
    ESP_LOGI(TAG, "Turning the white LED %s!", white_state == true ? "ON" : "OFF");
}

void all_off() {
    blue_state = 0;
    gpio_set_level(BLUE_GPIO, blue_state);
    white_state = 0;
    gpio_set_level(WHITE_GPIO, white_state);
}
/* Includes */
#ifndef LIGHT_H
#define LIGHT_H

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"

void light_init();
void all_off();
void blue_toggle();
void white_toggle();

#endif 
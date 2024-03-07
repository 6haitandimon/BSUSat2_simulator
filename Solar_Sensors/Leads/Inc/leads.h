#ifndef LEDS_H
#define LEDS_H

#include "WS2812b_config.h" //change
#include "../../Core/Inc/main.h"

#define NUM_LEDS     2 // change


void ws2812b_init();
void WS2812_Send(int pixelNum, uint8_t red, uint8_t green, uint8_t blue);
void WS2812_Reset();
#endif //LEDS_H

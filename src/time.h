#pragma once
#include "stdint.h"

// Wait for x miliiseconds, function use timer4
void delay(uint16_t ms);

// Wait for x microseconds, function dont use any timer, only loop.
// Function is not accurate. Around 100micro is most accurate.
void delay_micro_no_tim(uint32_t micros);

// Wait for x*4 microseconds, function use timer4.
// Function work from 4μs to 996s. Multiply input times 4 to get desire μs.
void delay_micro(uint8_t micros);

// Return time from power on in milliseconds. Up to 50days.
uint32_t millis(void);

void TIM4_UPD_OVF_IRQHandler() __interrupt(23);
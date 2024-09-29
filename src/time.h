#pragma once
#include "stdint.h"

typedef enum {
    CPU16Mhz,
    CPU8Mhz,
    CPU4Mhz,
    CPU2Mhz
}   cpu_clock;

// Wait for x miliiseconds, function use timer4
void delay(uint16_t ms);

// Wait for x microseconds, function dont use any timer, only loop.
// Function is not accurate. Around 100micro is most accurate.
void delay_micro_no_tim(uint32_t micros);

// Wait for x*4 microseconds, function use timer4.
// Function work from 4μs to 996s. Multiply input times 4 to get desire μs.
void delay_micro(uint8_t micros);

// Initialize timer4 for millis.
void TIM4_Config(cpu_clock);

// Return time from power on in milliseconds. Up to 50days.
uint32_t millis(void);

// Interrupt for timer4, for sdcc must be 'public'
void TIM4_UPD_OVF_IRQHandler() __interrupt(23);

// Start software stopwatch, take 0-3 channel
void stopwatch_start(uint8_t channel);

// Return stopwatch time in miliiseconds, take 0-3 channel
uint32_t stopwatch_stop(uint8_t channel);

// Cingure watchdog
void IWDG_Config();
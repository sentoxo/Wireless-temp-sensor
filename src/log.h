#pragma once
#include "stdint.h"

#define debugOnSerial() UART1_SendData8('$'); while (!(UART1->SR & UART1_SR_TC)) {};

// Print to serial1 using sprintf
void log(const char* pStr, ...);

// Initialize serial.
void UART_Config(uint32_t baudRate);

void UART1_TX_IRQHandler() __interrupt(17);
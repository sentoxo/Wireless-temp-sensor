#pragma once
#include "stdint.h"

// Print to serial1 using sprintf
void log(const char* pStr, ...);

// Initialize serial.
void UART_Config(uint32_t baudRate);

void UART1_TX_IRQHandler() __interrupt(17);
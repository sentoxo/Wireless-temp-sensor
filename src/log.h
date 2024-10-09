#pragma once
#include "stdint.h"

// Initialize serial.
void UART_Config(uint32_t baudRate);

void log_puts(const char* msg);

#ifdef USELOG
// Print to serial1 using sprintf
void log(const char* pStr, ...);

void UART1_TX_IRQHandler() __interrupt(17);
#endif
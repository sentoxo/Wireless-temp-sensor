#pragma once
#include <stdarg.h>
#include "stdio.h"
#include "stm8s.h"
#include "string.h"


void log(const char* pStr, ...);

void UART_Config(uint32_t baudRate);

void UART1_TX_IRQHandler() __interrupt(17);
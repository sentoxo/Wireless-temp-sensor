#pragma once
#include "stm8s.h"

#define DS18B20GPIOx GPIOD
#define DS18B20POWER GPIO_PIN_1
#define DS18B20DATA  GPIO_PIN_2

enum Ds18b20_errno {
    INIT_NO_RESPONSE = 1,
    REINIT_NO_RESPONSE = 2,
    LINE_HIGH = 4,
    LINE_LOW = 8,
    TEMP_NOT_VALID = 16,
    CONVERT_T_TIME_OUT = 32
};
extern enum Ds18b20_errno ds18b20_errno;

bool ds18b20_init();

void ds18b20_powerdown();

void ds18b20_read_scrachtpad();

void ds18b20_write_scratchpad(uint8_t Th, uint8_t Tl, uint8_t Config);

void ds18b20_convert_temp();

int16_t ds18b20_get_temp();

void ds18b20_read_rom();

void ds18b20_log_scratchpad();

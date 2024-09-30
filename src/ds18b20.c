#include "ds18b20.h"
#include "stm8s_gpio.h"
#include "time.h"

#define Skip_Rom         0xCC
#define Convert_T        0x44
#define Read_Scratchpad  0xBE
#define Write_Scratchpad 0x4E
#define Read_Rom         0x33

extern void log(const char* pStr, ...);

static uint8_t scratchpad[9];
static uint8_t rom[8];
enum Ds18b20_errno ds18b20_errno = 0;

int16_t
ds18b20_get_temp() {
    int16_t temp;
    int16_t raw_temp = (scratchpad[1] << 8 | scratchpad[0]);
    if (raw_temp & 0x8000) {
        raw_temp = (~raw_temp + 1) & 0xFFFF;
        temp = -((raw_temp >> 4) * 100 + (raw_temp & 0xF) * 100 / 16); //negative temp
    } else {
        temp = ((raw_temp >> 4) * 100 + (raw_temp & 0xF) * 100 / 16);
    }
    if (-5500 <= temp <= 12500) {
        return temp;
    } else {
        ds18b20_errno |= TEMP_NOT_VALID;
        return -10000;
    }
}

static bool
ds18b20_reinit() {
    bool ret = FALSE;
    GPIO_WriteLow(DS18B20GPIOx, DS18B20DATA); //Reset Pulse
    delay_micro(125);
    GPIO_WriteHigh(DS18B20GPIOx, DS18B20DATA);
    delay_micro(10);
    if (!(GPIO_ReadInputPin(DS18B20GPIOx, DS18B20DATA))) {
        ret = TRUE;
    }
    delay_micro(50);
    return ret;
}

bool
ds18b20_init() {
    ds18b20_errno = 0;
    GPIO_Init(DS18B20GPIOx, DS18B20POWER, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(DS18B20GPIOx, DS18B20DATA, GPIO_MODE_OUT_OD_HIZ_SLOW);

    GPIO_WriteLow(DS18B20GPIOx, DS18B20POWER); // Power reset
    delay(5);
    GPIO_WriteHigh(DS18B20GPIOx, DS18B20POWER);
    delay_micro(100);
    if (ds18b20_reinit()) {
        return TRUE;
    } else {
        ds18b20_errno |= INIT_NO_RESPONSE;
        return FALSE;
    }
}

static void
ds18b20_send_byte(uint8_t data) {
    disableInterrupts();
    uint8_t mask = 1u;
    for (uint8_t i = 0; i < 8; i++) {
        if (data & mask) {
            GPIO_WriteLow(DS18B20GPIOx, DS18B20DATA);
            //delay_micro(1);
            GPIO_WriteHigh(DS18B20GPIOx, DS18B20DATA);
            delay_micro(10);
        } else {
            GPIO_WriteLow(DS18B20GPIOx, DS18B20DATA);
            delay_micro(12);
            GPIO_WriteHigh(DS18B20GPIOx, DS18B20DATA);
            delay_micro(3);
        }
        mask = mask << 1;
    }
    enableInterrupts();
}

static uint8_t
ds18b20_read_byte() {
    disableInterrupts();
    uint8_t ret = 0;
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t bit = 0;
        GPIO_WriteLow(DS18B20GPIOx, DS18B20DATA);
        GPIO_WriteHigh(DS18B20GPIOx, DS18B20DATA);
        //No time to lose at 2Mhz
        //if(GPIO_ReadInputPin(DS18B20GPIOx, DS18B20DATA)){
        if ((DS18B20GPIOx->IDR & (uint8_t)DS18B20DATA)) {
            bit = 1;
        }
        ret = ret | (bit << i);
        delay_micro(15);
    }
    enableInterrupts();
    return ret;
}

void
ds18b20_read_scrachtpad() {
    ds18b20_send_byte(Skip_Rom);
    delay_micro(5);
    ds18b20_send_byte(Read_Scratchpad);
    delay_micro(10);
    for (uint8_t i = 0; i < 9; i++) {
        scratchpad[i] = ds18b20_read_byte();
    }

    uint8_t checkFF = 0;
    uint8_t check00 = 0;
    for (uint8_t n = 0; n < 9; n++) {
        if (scratchpad[n] == 0xFF) {
            checkFF++;
        }
        if (scratchpad[n] == 0x00) {
            check00++;
        }
    }
    if (checkFF > 7) {
        ds18b20_errno |= LINE_HIGH;
    } else if (check00 > 7) {
        ds18b20_errno |= LINE_LOW;
    }

    if (!ds18b20_reinit()) {
        ds18b20_errno |= REINIT_NO_RESPONSE;
    }
}

void
ds18b20_write_scratchpad(uint8_t Th, uint8_t Tl, uint8_t Config) {
    ds18b20_send_byte(Skip_Rom);
    delay_micro(5);
    ds18b20_send_byte(Write_Scratchpad);
    delay_micro(5);
    ds18b20_send_byte(Th);
    delay_micro(5);
    ds18b20_send_byte(Tl);
    delay_micro(5);
    ds18b20_send_byte(Config);
    delay_micro(10);
    if (!ds18b20_reinit()) {
        ds18b20_errno |= REINIT_NO_RESPONSE;
    }
}

void
ds18b20_convert_temp() {
    ds18b20_send_byte(Skip_Rom);
    delay_micro(5);
    ds18b20_send_byte(Convert_T);
    uint32_t ts = millis();
    while (!ds18b20_read_byte()) {
        //My ds18b20 converts in 520ms, datasheet say max time is 750ms
        if ((millis() - ts) > 800) {
            ds18b20_errno |= CONVERT_T_TIME_OUT;
            break;
        }
    }
    if (!ds18b20_reinit()) {
        ds18b20_errno |= REINIT_NO_RESPONSE;
    }
}

void
ds18b20_read_rom() {
    ds18b20_send_byte(Read_Rom);
    delay_micro(5);
    for (uint8_t i = 0; i < 8; i++) {
        rom[i] = ds18b20_read_byte();
    }
    ds18b20_reinit();
    log(" %x,%x,%x,%x,%x,%x,%x,%x\n\r", rom[0], rom[1], rom[2], rom[3], rom[4], rom[4], rom[6], rom[5], rom[7]);
}

void
ds18b20_log_scratchpad() {
    log(" %x,%x,%x,%x,%x,%x,%x,%x,%x\n\r", scratchpad[0], scratchpad[1], scratchpad[2], scratchpad[3], scratchpad[4],
        scratchpad[4], scratchpad[6], scratchpad[5], scratchpad[8]);
}
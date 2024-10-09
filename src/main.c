#include "ds18b20.h"
#include "log.h"
#include "nrf24l01.h"
#include "stdio.h"
#include "stdlib.h"
#include "stm8s_conf.h"
#include "time.h"

#define sleepMultiplayar 10 // n*30s of sleep

union Ascending_number {
    uint32_t t32;
    uint8_t t8[4];
} ascending_number;

struct Payload {
    int16_t temp;
    uint16_t id;
    uint16_t ds18b20_errno;
    uint32_t millis;
} payload;

/*
void
EEPROM_WriteByte(uint16_t Address, uint8_t Data) {
    // Unlock EEPROM for writing
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    // Wait until EEPROM is ready
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == NRF_RESET)
        ;
    // Write the byte to the EEPROM at the specified address
    FLASH_ProgramByte(Address + (uint16_t)0x4000, Data);
    // Lock EEPROM to prevent further accidental writing
    FLASH_Lock(FLASH_MEMTYPE_DATA);
}

uint8_t
EEPROM_ReadByte(uint16_t Address) {
    // Read the byte from the EEPROM at the specified address
    return FLASH_ReadByte(Address + (uint16_t)0x4000);
}
*/
void
blink(void) {
    GPIO_WriteLow(GPIOB, GPIO_PIN_5);
    delay_micro(125);
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
}

void
radio_init(void) {
    nrf24_device(TRANSMITTER, NRF_RESET);
    nrf24_address_width(5);
    nrf24_rf_channel(90);
    nrf24_rf_datarate(250);
    nrf24_datapipe_ptx(1);
    //nrf24_dynamic_ack(ENABLE);
    //nrf24_automatic_retransmit_setup(1500, 5);
    nrf24_rf_power(0); // 0 6 12 18
}

void
radio_test(uint16_t n_packets, uint8_t every_ms) {
    log_puts("TX-GO\r\n");
    ascending_number.t32 = 1;
    uint32_t t = millis();
    for (; n_packets > 0;) {
        if (millis() >= (t + every_ms)) {
            t = millis();
            n_packets--;
            blink();
            while (nrf24_transmit(ascending_number.t8, 4, NO_ACK_MODE) == TRANSMIT_FAIL) {
                log_puts("Failed TX\r\n");
                delay(500);
            }
            while (nrf24_transmit_status() == TRANSMIT_IN_PROGRESS)
                ;
            ascending_number.t32++;
            log_puts("N\r\n");
        }
    }
    log_puts("TX-END\r\n");
}

void
send_payload() {
    blink();
    nrf24_transmit((uint8_t*)payload, sizeof(struct Payload), NO_ACK_MODE);
    uint32_t t = millis();
    while (nrf24_transmit_status() == TRANSMIT_IN_PROGRESS) {
        if (millis() > (t + 5)) {
            break;
        }
    }
}

main() {
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOD);
    GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);

    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV8); //CPU clock 2Mhz
    TIM4_Config(CPU2Mhz);
    //UART_Config(115200);
    //IWDG_Config();
    enableInterrupts();
    /*
    uint32_t start_counter = EEPROM_ReadByte(0);
    EEPROM_WriteByte(0, (uint8_t)start_counter + 1);
    log("--START nr.%u, compiled at %s\n\r", start_counter, __TIME__);
    if (RST_GetFlagStatus(RST_FLAG_IWDGF) == SET) {
        RST_ClearFlag(RST_FLAG_IWDGF);
        log("Start after watchdog reset\n\r");
    }
*/
    //Lower power consumption at sleep
    CLK_SlowActiveHaltWakeUpCmd(ENABLE);
    FLASH->CR1 |= (uint8_t)0x04; //AHALT: Power-down in Active-halt mode
    //FLASH_SetLowPowerMode(FLASH_LPMODE_POWERDOWN);

    AWU_Init(AWU_TIMEBASE_30S);
    AWU_Cmd(ENABLE);

    payload.id = 0;
    uint32_t t = millis();

    while (1) {
        payload.id++;

        if (ds18b20_init()) {
            ds18b20_convert_temp();
            ds18b20_read_scrachtpad();
        }
        ds18b20_powerdown();
        if (ds18b20_errno) {
            payload.ds18b20_errno = ds18b20_errno;
            payload.temp = -9999;
        } else {
            payload.temp = ds18b20_get_temp();
            payload.ds18b20_errno = 0;
        }
        payload.millis = millis();
        radio_init();
        for (uint8_t i = 0; i < 5; i++) {
            send_payload();
            delay(5);
        }
        nrf24_mode(POWER_DOWN);

        for (uint8_t i = 0; i < sleepMultiplayar; i++) {
            halt();
        }
    }
}

void
AWU_IRQHandler() __interrupt(1) {
    AWU_GetFlagStatus();
}
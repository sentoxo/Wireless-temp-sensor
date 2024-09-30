#include "ds18b20.h"
#include "log.h"
#include "stdio.h"
#include "stdlib.h"
#include "stm8s_conf.h"
#include "time.h"

void
EEPROM_WriteByte(uint16_t Address, uint8_t Data) {
    // Unlock EEPROM for writing
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    // Wait until EEPROM is ready
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
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

void
blink(void) {
    GPIO_WriteLow(GPIOB, GPIO_PIN_5);
    delay_micro(249); //~1ms
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
}

main() {
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOD);
    GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);

    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV8);
    TIM4_Config(CPU2Mhz);
    UART_Config(115200);
    //IWDG_Config();
    enableInterrupts();

    uint32_t start_counter = EEPROM_ReadByte(0);
    EEPROM_WriteByte(0, (uint8_t)start_counter + 1);
    log("--START nr.%u, compiled at %s\n\r", start_counter, __TIME__);
    if (RST_GetFlagStatus(RST_FLAG_IWDGF) == SET) {
        RST_ClearFlag(RST_FLAG_IWDGF);
        log("Start after watchdog reset\n\r");
    }

    delay(800);

    //AWU_Init(AWU_TIMEBASE_2S);
    //AWU_Cmd(ENABLE);

    uint32_t t = millis();
    while (1) {
        //IWDG_ReloadCounter();
        /*
        if(millis()>=(t+1000)){
            t = millis();
            log("01234567890\n\r");
        }
        */
        delay(50);

        if (ds18b20_init()) {
            blink();
            ds18b20_convert_temp();
            ds18b20_read_scrachtpad();
        }
        if (ds18b20_errno) {
            log("ds18b20 exception: %d \n\r", ds18b20_errno);
        } else {
            int16_t temp = ds18b20_get_temp();
            if (temp < 0) {
                log("Temp: -%d.%02d\n\r", abs(temp / 100), abs(temp % 100));
            } else {
                log("Temp: %d.%02d\n\r", temp / 100, abs(temp % 100));
            }
        }

        /*
        for (uint8_t i = 0; i < sleepMultiplayar; i++){
            halt();
        }
        */
    }
}

void
AWU_IRQHandler() __interrupt(1) {
    AWU_GetFlagStatus();
}
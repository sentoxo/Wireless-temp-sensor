#include "stm8s_conf.h"
#include "stdio.h"
#include "log.h"
#include "time.h"


void EEPROM_WriteByte(uint16_t Address, uint8_t Data){
    // Unlock EEPROM for writing
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    // Wait until EEPROM is ready
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET);
    // Write the byte to the EEPROM at the specified address
    FLASH_ProgramByte(Address+(uint16_t)0x4000, Data);
    // Lock EEPROM to prevent further accidental writing
    FLASH_Lock(FLASH_MEMTYPE_DATA);
}

uint8_t EEPROM_ReadByte(uint16_t Address){
    // Read the byte from the EEPROM at the specified address
    return FLASH_ReadByte(Address+(uint16_t)0x4000);
}


main(){
    GPIO_DeInit(GPIOB);
    GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
    TIM4_Config();
    UART_Config(115200);
    enableInterrupts();

    uint32_t start_counter = EEPROM_ReadByte(0);
    EEPROM_WriteByte(0, (uint8_t)start_counter+1);
    log("--START nr.%u, compiled at %s\n\r", start_counter, __TIME__);
    
    uint32_t t = millis();
    uint32_t timer1, timer2;
    while (1)
    {
        if(millis()>=(t+1000)){
            t = millis();
            timer1 = millis();
            log("hejo\n\r");
            timer2 = millis();
            log("Timing: %u\n\r", timer2-timer1);
            GPIO_WriteReverse(GPIOB, GPIO_PIN_5);
        }

    }
}


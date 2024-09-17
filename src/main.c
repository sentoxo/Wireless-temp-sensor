#include "stm8s_conf.h"
#include "stdio.h"
#include "log.h"

volatile uint32_t millis_counter = 0; 
uint32_t millis(void);

void delay(uint16_t ms) {
    uint32_t t = millis();
    while(millis()<(t+ms)){}
}

void delay_micro_no_tim(uint32_t micros){
    //Longer below 100mikro and faster above it
    micros = micros>>1; // faster -> micros=/2
    for (uint32_t i = 0; i <= micros; i++){
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
        __asm__ ("nop");
    }
}

void delay_micro(uint8_t micros){
    uint8_t start = (uint8_t)(TIM4->CNTR);
    for (;;) {
        uint8_t now = (uint8_t)(TIM4->CNTR);      
        uint8_t elapsed = now - start;  
        if (elapsed >= micros)                    
            return;
    }
}

void TIM4_Config(void){
    // Enable TIM4 clock
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);
    TIM4_TimeBaseInit(TIM4_PRESCALER_64, 251); //1ms
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    // Enable TIM4 update interrupt
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    // Enable TIM4
    TIM4_Cmd(ENABLE);
}

uint32_t millis(void){
    uint32_t millis_value;
    // Disable interrupts to ensure atomic read of millis_counter
    disableInterrupts();
    millis_value = millis_counter;
    enableInterrupts();
    return millis_value;
}

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

void TIM4_UPD_OVF_IRQHandler() __interrupt(23){
    // Increment the milliseconds counter
    millis_counter++;
    // Clear TIM4 update interrupt flag
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}


main(){
    GPIO_DeInit(GPIOB);
    GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
    TIM4_Config();
    UART_Config(115200);
    enableInterrupts();

    uint8_t start_counter = EEPROM_ReadByte(0);
    EEPROM_WriteByte(0, start_counter+1);
    log("--START nr.%u, compiled at %s\n\r", start_counter, __TIME__);
    
    uint32_t t = millis();
    
    while (1)
    {
        if(millis()>=(t+1000)){
            log("hejo\n\r");
            GPIO_WriteReverse(GPIOB, GPIO_PIN_5);
            t = millis();
        }

    }
}


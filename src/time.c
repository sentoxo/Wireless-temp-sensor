#include "time.h"
#include "stm8s.h"

static volatile uint32_t millis_counter = 0; 
static uint32_t stopwatchTime[4];

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
    TIM4_TimeBaseInit(TIM4_PRESCALER_64, 249); //1ms
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    // Enable TIM4 update interrupt
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    // Enable TIM4
    TIM4_Cmd(ENABLE);
}

void TIM4_UPD_OVF_IRQHandler() __interrupt(23){
    // Increment the milliseconds counter
    millis_counter++;
    // Clear TIM4 update interrupt flag
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}

uint32_t millis(void){
    uint32_t millis_value;
    // Disable interrupts to ensure atomic read of millis_counter
    disableInterrupts();
    millis_value = millis_counter;
    enableInterrupts();
    return millis_value;
}

void stopwatch_start(uint8_t channel){
    if(channel < 4){
        stopwatchTime[channel] = millis();
    }
}

uint32_t stopwatch_stop(uint8_t channel){
    if(channel < 4){
        return millis() - stopwatchTime[channel];
    }else{
        return 0;
    }
}

void IWDG_Config(){
    IWDG_Enable();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetReload(255);
    IWDG_ReloadCounter();
}
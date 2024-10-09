//#include "nrf24l01.h"
#include "stm8s.h"

extern void delay(uint16_t ms);

/*start of low level functions, specific to the mcu and compiler*/
/*delay in miliseconds*/
void
delay_function(uint32_t duration_ms) {
    return delay(duration_ms);
}

/*contains all SPI configuations, such as pins and control registers*/
/*SPI control: master, interrupts disabled, clock polarity low when idle, clock phase falling edge, clock up tp 1 MHz*/
void
SPI_Initializer() {
    // Enable SPI clock
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, ENABLE);
    // Configure SPI pins (SCK, MISO, MOSI)
    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST); // SCK
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST); // MOSI
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);      // MISO
    // SPI configuration
    SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_2, SPI_MODE_MASTER, SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_1EDGE,
             SPI_DATADIRECTION_2LINES_FULLDUPLEX, SPI_NSS_SOFT, 0x07);
    // Enable SPI
    SPI_Cmd(ENABLE);
}

/*contains all CSN and CE pins gpio configurations, including setting them as gpio outputs and turning SPI off and CE '1'*/
void
pinout_Initializer() {
    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST); // CE
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST); // CSN
    GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
    GPIO_WriteHigh(GPIOC, GPIO_PIN_4);
}

/*CSN pin manipulation to high or low (SPI on or off)*/
void
nrf24_SPI(uint8_t input) {
    if (input) {
        GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
    } else {
        GPIO_WriteLow(GPIOC, GPIO_PIN_3);
    }
}

/*1 byte SPI shift register send and receive routine*/
uint8_t
SPI_send_command(uint8_t command) {
    // Wait until SPI is ready to transmit
    while (SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET) {};
    // Send byte
    SPI_SendData(command);
    // Wait until the transmission is complete
    while (SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET) {};
    // Return received byte (even if unused)
    return SPI_ReceiveData();
}

/*CE pin maniplation to high or low*/
void
nrf24_CE(uint8_t input) {
    if (input) {
        GPIO_WriteHigh(GPIOC, GPIO_PIN_4);
    } else {
        GPIO_WriteLow(GPIOC, GPIO_PIN_4);
    }
}

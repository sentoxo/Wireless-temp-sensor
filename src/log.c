#include "log.h"
#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "stm8s.h"
#include "time.h"

#define TxLen 100

static uint8_t TxBuffer1[TxLen];
static uint8_t bufove = 0;
static const char* timeFormat= "%03u:%02u:%02u.%03u#";
static const char* bo = "Buffer overflow!\n\r";
static volatile uint8_t TxCounter1 = 0;
static volatile uint16_t RxCounter1 = 0;
static volatile bool RxFull = FALSE;
static volatile bool TxActive = FALSE;


void log(const char* pStr, ...){
    va_list ap;
    uint16_t counter = 0;
    while(TxActive){}; // Wait until the previous message is sent

    sprintf(TxBuffer1, timeFormat, 
            (uint16_t)(millis()/3600000), (uint16_t)(millis()%3600000/60000), 
            (uint16_t)(millis()%60000/1000), (uint16_t)(millis()%1000));
    //Pass arguments to sprintf and print to buffer after timestamp
    va_start(ap, pStr);
    counter += vsprintf((char*)(TxBuffer1+14), pStr, ap) + 14; 
    va_end(ap);

    if(counter >= TxLen){
        strcpy((char*)(TxBuffer1+14), bo);
    }

    UART1_ITConfig(UART1_IT_TXE, ENABLE); //Enable sending message by interupts
    TxActive = TRUE;
}

void UART_Config(uint32_t baudRate){
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1, ENABLE);
    UART1_DeInit();
    UART1_Init(baudRate, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    UART1_ITConfig(UART1_IT_TXE, ENABLE);
    //UART1_ITConfig(UART1_IT_RXNE, ENABLE);
    UART1_Cmd(ENABLE);
}

void UART1_TX_IRQHandler() __interrupt(17){
    UART1_SendData8(TxBuffer1[TxCounter1++]);
    if (TxCounter1 == TxLen || TxBuffer1[TxCounter1] == '\0'){
        TxCounter1 = 0;
        UART1_ITConfig(UART1_IT_TXE, DISABLE);
        TxActive = FALSE;
    }
}

/*
void UART1_RX_IRQHandler() __interrupt(18){
    char x = UART1_ReceiveData8();
    UART1_SendData8(x);
    EEPROM_WriteByte(RxCounter1+RXOFFSET, x);
    ++RxCounter1;
    if(RxCounter1 > 8){
        RxFull = TRUE;
    }
}
*/
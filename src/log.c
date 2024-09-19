#include "log.h"
#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "stm8s.h"
#include "time.h"

#define TxLen 100
//#define useSprintf

static uint8_t TxBuffer1[TxLen];
static uint8_t bufove = 0;
static const char* timeFormat= "%03u:%02u:%02u.%03u# ";
static const char* bo = "Buffer overflow!\n\r";
static volatile uint8_t TxCounter1 = 0;
static volatile uint16_t RxCounter1 = 0;
static volatile bool RxFull = FALSE;
static volatile bool TxActive = FALSE;

#ifndef useSprintf
static uint16_t strcopy(char* s, const char* format, uint16_t counter){
    uint16_t retN = 0;
    while(counter && *format){
        *s = *format;
        counter--;
        retN++;
        format++;
        s++;
    }
    return retN;
}

static uint8_t u_to_str(uint32_t num, char* buf) {
    uint8_t len = 0;
    do{
        buf[len++] = (num%10) + '0';
        num /= 10;
    } while(num>0);
    return len;
}
    
static int _vsnprintf_nano(char* s, uint16_t n, const char* format, va_list ap ){
    uint16_t counter = n;
    uint8_t width = 0;

    while(*format){
        if(*format == '%'){
            format++;
            if(*format == '0'){
                format++;
                if(*format >= '1' && *format < '9'){
                    width = *format - '0';
                    format++;
                }
            }
            if(*format == '%'){
                *s = *format;
                s++;
                counter--;
            }else if(*format == 's'){
                uint8_t tmp;
                tmp = strcopy(s, va_arg(ap, const char*), counter);
                s += tmp;
                counter -= tmp;
            }else if(*format == 'u'){
                uint32_t num = va_arg(ap, uint32_t);
                char numStr[11];
                uint8_t numLen = u_to_str(num, numStr);
                if(numLen < width && counter >= (width - numLen)){
                    uint8_t padding = width - numLen;
                    for (uint8_t i = 0; i < padding && counter; i++, counter--){
                        s[i] = '0';
                    }
                    s += padding;
                }
                for (int i = numLen-1; i >= 0 && counter > 1; i--){
                    *s++ = numStr[i];
                    counter--;
                }
                
            }
        }else{
            *s = *format;
            s++;
            counter--;
        }
        format++;
        if(!counter){
            *(--s) = '\0'; //Terminate last char in buffer
            return n;
        }
    }
    *s = '\0';
 
    return n - counter;
}

int snprintf_nano(char* s, uint16_t n, const char* format, ...){
    va_list ap;
    va_start(ap, format);
    int result = _vsnprintf_nano(s, n, format, ap);
    va_end(ap);
    return result;
}
#endif

void log(const char* pStr, ...){
    va_list ap;
    va_start(ap, pStr);
    uint16_t counter = 0;
    while(TxActive){}; // Wait until the previous message is sent

    #ifdef useSprintf
    counter += sprintf(TxBuffer1, timeFormat, 
                (uint16_t)(millis()/3600000), (uint16_t)(millis()%3600000/60000), 
                (uint16_t)(millis()%60000/1000), (uint16_t)(millis()%1000));
    counter += vsprintf((char*)(TxBuffer1+14), pStr, ap) + 14; 
    #else
    counter += snprintf_nano(TxBuffer1, 16, timeFormat,
                (uint32_t)(millis()/3600000), (uint32_t)(millis()%3600000/60000), 
                (uint32_t)(millis()%60000/1000), (uint32_t)(millis()%1000));
    counter += _vsnprintf_nano((char*)(TxBuffer1) + counter, TxLen-counter-1, pStr, ap);
    #endif
    
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
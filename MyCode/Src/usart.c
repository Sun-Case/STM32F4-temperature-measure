#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "esp8266.h"
#include "rtc.h"

/*!
 * printf 重定向
 * 注意，重定向后使用 UART1 传输输出，需确保 UART1 已启用，否则会假死
 */
void printf_redirect() {
    int static state = 0;
    if (state) {
        return;
    }
    state = 1;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Speed=GPIO_High_Speed,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_Pin=GPIO_Pin_9 | GPIO_Pin_10,
            .GPIO_Mode=GPIO_Mode_AF,
    };
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct = {
            .USART_BaudRate=115200,
            .USART_Mode=USART_Mode_Tx | USART_Mode_Rx,
            .USART_Parity=USART_Parity_No,
            .USART_WordLength=USART_WordLength_8b,
            .USART_StopBits=USART_StopBits_1,
            .USART_HardwareFlowControl=USART_HardwareFlowControl_None,
    };
    USART_Init(USART1, &USART_InitStruct);

    USART_Cmd(USART1, ENABLE);
}

void usart3_init(uint32_t baud) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11,
            .GPIO_Mode=GPIO_Mode_AF,
            .GPIO_Speed=GPIO_High_Speed,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_PuPd=GPIO_PuPd_NOPULL,
    };
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    //配置USART1的相关参数：波特率、数据位、校验位
    USART_InitTypeDef USART_InitStruct = {
            .USART_BaudRate=baud,
            .USART_WordLength=USART_WordLength_8b,
            .USART_StopBits=USART_StopBits_1,
            .USART_Parity=USART_Parity_No,
            .USART_HardwareFlowControl=USART_HardwareFlowControl_None,
            .USART_Mode=USART_Mode_Rx | USART_Mode_Tx,
    };
    USART_Init(USART3, &USART_InitStruct);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=USART3_IRQn,
            .NVIC_IRQChannelPreemptionPriority=0,
            .NVIC_IRQChannelSubPriority=0,
            .NVIC_IRQChannelCmd=ENABLE,
    };
    NVIC_Init(&NVIC_InitStruct);

    USART_Cmd(USART3, ENABLE);
}

void usart3_send_str(char *str) {
    char *p = str;

    while (*p != '\0') {
        USART_SendData(USART3, *p);

        p++;

        //等待数据发送成功
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_ClearFlag(USART3, USART_FLAG_TXE);
    }
}

void usart3_send_bytes(uint8_t *buf, uint32_t len) {
    uint8_t *p = buf;

    while (len--) {
        USART_SendData(USART3, *p);

        p++;

        //等待数据发送成功
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_ClearFlag(USART3, USART_FLAG_TXE);
    }
}

void USART3_IRQHandler(void) {
    uint8_t d = 0;

    //检测是否接收到数据
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        d = USART_ReceiveData(USART3);

        static int yyyy = 0, mm = 0, dd = 0, hh = 0, MM = 0, ss = 0, wd = 0;

        g_esp8266_rx_buf[g_esp8266_rx_cnt++] = d;

        if (g_esp8266_rx_cnt >= sizeof g_esp8266_rx_buf) {
            g_esp8266_rx_end = 1;

            g_esp8266_rx_cnt = 0;
            g_esp8266_rx_end = 0;
        }

        if (sscanf(g_esp8266_rx_buf, "%d-%d-%d %d:%d:%d %d", &yyyy, &mm, &dd, &hh,
                   &MM, &ss,
                   &wd) ==
            7) {
            printf("Recv Time: %d-%d-%d %d:%d:%d %d\n", yyyy, mm, dd, hh, MM, ss, wd);
            Rtc_SetDate(yyyy - 2000, mm, dd, wd);
            Rtc_SetTime(hh, MM, ss);
            bzero(g_esp8266_rx_buf, sizeof(g_esp8266_rx_buf));
//            esp8266_disconnect_server();
        }

#if EN_DEBUG_ESP8266
        //将接收到的数据返发给PC
        USART_SendData(USART1, d);
        //while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
#endif
        //清空标志位，可以响应新的中断请求
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

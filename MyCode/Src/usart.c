#include "stm32f4xx.h"
#include "usart.h"

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

#include "stm32f4xx.h"
#include "key.h"

void Key_Init(KEY_TypeDef keyX) {
    u32 RCC_AHB1Periph;
    GPIO_TypeDef *GPIOx;
    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Mode=GPIO_Mode_IN,
            .GPIO_Speed=GPIO_Speed_50MHz,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_OType=GPIO_OType_PP,
    };

    switch (keyX) {
        case KEY_0:
            GPIOx = GPIOA;
            RCC_AHB1Periph = RCC_AHB1Periph_GPIOA;
            break;
        case KEY_1:
        case KEY_2:
        case KEY_3:
            GPIOx = GPIOE;
            RCC_AHB1Periph = RCC_AHB1Periph_GPIOE;
            break;
        default:
            return;
    }

    switch (keyX) {
        case KEY_0:
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
            break;
        case KEY_1:
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
            break;
        case KEY_2:
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
            break;
        case KEY_3:
            GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
            break;
    }

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
    GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Key_NVIC_Init(KEY_TypeDef keyX, EXTITrigger_TypeDef triggerType, u8 preemptionPriority, u8 subPriority) {
    EXTI_InitTypeDef EXTI_InitStruct = {
            .EXTI_LineCmd=ENABLE,
            .EXTI_Trigger=triggerType,
            .EXTI_Mode=EXTI_Mode_Interrupt
    };
    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannelCmd=ENABLE,
            .NVIC_IRQChannelPreemptionPriority=preemptionPriority,
            .NVIC_IRQChannelSubPriority=subPriority
    };
    uint8_t EXTI_PortSourceGPIOx;
    uint8_t EXTI_PinSourcex;

    switch (keyX) {
        case KEY_0:
            EXTI_InitStruct.EXTI_Line = EXTI_Line0;
            NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
            EXTI_PortSourceGPIOx = EXTI_PortSourceGPIOA;
            EXTI_PinSourcex = EXTI_PinSource0;
            break;
        case KEY_1:
            EXTI_InitStruct.EXTI_Line = EXTI_Line2;
            NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
            EXTI_PortSourceGPIOx = EXTI_PortSourceGPIOE;
            EXTI_PinSourcex = EXTI_PinSource2;
            break;
        case KEY_2:
            EXTI_InitStruct.EXTI_Line = EXTI_Line3;
            NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
            EXTI_PortSourceGPIOx = EXTI_PortSourceGPIOE;
            EXTI_PinSourcex = EXTI_PinSource3;
            break;
        case KEY_3:
            EXTI_InitStruct.EXTI_Line = EXTI_Line4;
            NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
            EXTI_PortSourceGPIOx = EXTI_PortSourceGPIOE;
            EXTI_PinSourcex = EXTI_PinSource4;
            break;
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOx, EXTI_PinSourcex);
    EXTI_Init(&EXTI_InitStruct);
    NVIC_Init(&NVIC_InitStruct);
}

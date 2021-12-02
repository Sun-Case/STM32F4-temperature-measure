#include "beep.h"
#include "sys.h"

void Beep_Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Mode=GPIO_Mode_OUT,
            .GPIO_Pin=GPIO_Pin_8,
            .GPIO_Speed=GPIO_High_Speed,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_OType=GPIO_OType_PP,
    };
    GPIO_Init(GPIOF, &GPIO_InitStruct);

    Beep_Off();
}

void Beep_Toggle() {
    GPIO_ToggleBits(GPIOF, GPIO_Pin_8);
}

void Beep_On() {
    PFout(8) = 1;
}

void Beep_Off() {
    PFout(8) = 0;
}

#include <stdio.h>
#include "main.h"

/*!
 * Project: STM32F4 GY-906 体温测量
 * Version: 0.0.3
 */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    // 重定向 printf
    printf_redirect();

    printf("\nStart init\n");

    // 定时器 初始化
    Delay_Init();
    // GY-906 初始化
    Gy906_Init();
    // RTC 初始化
    Rtc_Init();
    // OLed 初始化
    Oled_Init();
    Oled_Fill(0x00);
    // 初始化 TIM3 定时器，显示时间
    Tim3_Init();

    printf("Init Finish\n");

    while (1) {
    }
}

#pragma clang diagnostic pop

void Tim3_Init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimBaseStruct = {
            .TIM_Prescaler=84 - 1,
            .TIM_Period=1000 - 1,
            .TIM_CounterMode=TIM_CounterMode_Up,
            .TIM_ClockDivision=TIM_CKD_DIV1,
    };
    TIM_TimeBaseInit(TIM3, &TIM_TimBaseStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=TIM3_IRQn,
            .NVIC_IRQChannelCmd=ENABLE,
            .NVIC_IRQChannelPreemptionPriority=0,
            .NVIC_IRQChannelSubPriority=0
    };
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}

__attribute__((unused)) void TIM3_IRQHandler() {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
        RTC_TimeTypeDef RTC_TimeStruct;
        RTC_DateTypeDef RTC_DateStruct;
        RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
        RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
        char buf[255] = {0};
        sprintf(buf, "20%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month,
                RTC_DateStruct.RTC_Date, RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes,
                RTC_TimeStruct.RTC_Seconds);
        Oled_ShowAscii(7, 0, buf, s6x8);

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

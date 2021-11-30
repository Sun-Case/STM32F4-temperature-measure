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

    printf("Init Finish\n");

    while (1) {
    }
}

#pragma clang diagnostic pop

#include <stdio.h>
#include "main.h"

/*!
 * Project: STM32F4 GY-906 体温测量
 * Version: 0.0.1
 */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    // 重定向 printf
    printf_redirect();

    printf("\nStart init\n");

    Delay_Init();

    printf("Init Finish\n");

    while (1) {}
}

#pragma clang diagnostic pop

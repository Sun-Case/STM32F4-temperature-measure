#include "stm32f4xx.h"
#include "delay.h"

/*!
 * 使用 SysTick 定时器来延时
 */

// 初始化
void Delay_Init() {
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

// 延时 us
void Delay_us(u32 delay) {
    u32 tmp;

    SysTick->LOAD = 21 * delay - 1;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= 0x01;

    do {
        tmp = SysTick->CTRL;
    } while ((tmp & 0x01) && !(tmp & (0x01 << 16)));

    SysTick->CTRL &= ~(0x01);
}

// 延时 ms
void Delay_ms(u32 delay) {
    u32 tmp;

    SysTick->LOAD = 21000 * delay - 1;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= 0x01;

    do {
        tmp = SysTick->CTRL;
    } while ((tmp & 0x01) && !(tmp & (0x01 << 16)));

    SysTick->CTRL &= ~(0x01);
}

// 延时 s
void Delay_s(u32 delay) {
    for (u32 i = 0; i < delay; i++) {
        Delay_ms(500);
        Delay_ms(500);
    }
}

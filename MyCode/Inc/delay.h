#ifndef TEMPLATE_DELAY_H
#define TEMPLATE_DELAY_H

#include "stm32f4xx.h"

void Delay_Init();

void Delay_us(u32 delay);

void Delay_ms(u32 delay);

void Delay_s(u32 delay);

#endif //TEMPLATE_DELAY_H

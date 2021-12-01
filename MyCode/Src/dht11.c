#include "dht11.h"
#include "delay.h"
#include "stm32f4xx.h"
#include "sys.h"

#define delay_ms(delay) Delay_ms(delay)
#define delay_us(delay) Delay_us(delay)

void Dht11_Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Pin=GPIO_Pin_9,
            .GPIO_Mode=GPIO_Mode_OUT,
            .GPIO_PuPd=GPIO_PuPd_NOPULL,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_Speed=GPIO_Speed_100MHz
    };
    GPIO_Init(GPIOG, &GPIO_InitStruct);

    PGout(9) = 1;
}

void dht11_Toggle(GPIOMode_TypeDef mode) {
    static GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Speed=GPIO_Speed_100MHz,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_PuPd=GPIO_PuPd_NOPULL,
            .GPIO_Pin=GPIO_Pin_9
    };
    GPIO_InitStruct.GPIO_Mode = mode;
    GPIO_Init(GPIOG, &GPIO_InitStruct);
}

int Dht11_Start() {
    u16 t = 0;

    dht11_Toggle(GPIO_Mode_OUT);
    PGout(9) = 1;
    delay_ms(1);
    PGout(9) = 0;
    delay_ms(20);
    PGout(9) = 1;
    delay_us(30);

    dht11_Toggle(GPIO_Mode_IN);

    while (PGin(9) == 1) {
        t++;
        delay_us(5);
        if (t >= 50) {
            return -1;
        }
    }
    delay_us(40);

    t = 0;
    while (PGin(9) == 0) {
        t++;
        delay_us(5);
        if (t >= 50) {
            return -2;
        }
    }
    delay_us(40);

    t = 0;
    while (PGin(9) == 1) {
        t++;
        delay_us(5);
        if (t >= 50) {
            return -3;
        }
    }
    return 0;
}

u8 Dht11_Read_Byte() {
    u8 data = 0x00;

    for (int i = 0; i < 8; i++) {
        while (PGin(9) == 0);
        data <<= 1;
        delay_us(40);
        if (PGin(9) == 1) {
            data |= 0x01;
            while (PGin(9) == 1);
        }
    }
    return data;
}

#undef delay_ms
#undef delay_us

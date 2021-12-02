#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "gy906.h"
#include "rtc.h"
#include "oled.h"
#include "key.h"
#include "dht11.h"
#include "esp8266.h"
#include "spi.h"
#include "w25q128.h"

void Tim3_Init();

void Tim4_Init();

void Tim2_Init();

void Esp8266_Init();

void Tim5_Init();

typedef struct {
    u8 check;
    u8 count;
    u8 offset;
    u8 T[32];
} TemperatureData;

void Save_Data();

u8 Build_Hash();

int Check();

#endif

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
    u32 check;
    u32 count;
    u32 offset;
    float T[32];
} TemperatureData;

void Show_Temperature();

void Add_Temperature(float t);

void Save_Data();

u32 Build_Hash();

int Check();

#endif

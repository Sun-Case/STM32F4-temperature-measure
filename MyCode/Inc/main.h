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

void Tim3_Init();

void Tim4_Init();

void Tim2_Init();

void Esp8266_Init();

void Tim5_Init();

#endif

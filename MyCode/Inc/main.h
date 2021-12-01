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

void Tim3_Init();

void Tim4_Init();

void Tim2_Init();

#endif

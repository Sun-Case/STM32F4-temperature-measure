#include "stm32f4xx.h"
#include "delay.h"
#include "rtc.h"

#define delay_ms(delay) Delay_ms(delay)

void Rtc_Init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    delay_ms(50);
    RTC_InitTypeDef RTC_InitStruct = {
            .RTC_HourFormat=RTC_HourFormat_24,
            .RTC_AsynchPrediv=0x7F,
            .RTC_SynchPrediv=0xFF,
    };
    RTC_Init(&RTC_InitStruct);
}

void Rtc_SetDate(u8 year, u8 month, u8 date, u8 weekDay) {
    RTC_DateTypeDef RTC_DataStruct = {
            .RTC_Year=year,
            .RTC_Month=month,
            .RTC_Date=date,
            .RTC_WeekDay=weekDay,
    };
    RTC_SetDate(RTC_Format_BIN, &RTC_DataStruct);
}

void Rtc_SetTime(u8 hours, u8 minutes, u8 seconds) {
    RTC_TimeTypeDef RTC_TimeStruct = {
            .RTC_H12=(hours >= 12) ? (RTC_H12_PM) : (RTC_H12_AM),
            .RTC_Hours=hours,
            .RTC_Minutes=minutes,
            .RTC_Seconds=seconds,
    };
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
}

#undef delay_ms

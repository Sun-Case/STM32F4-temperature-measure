#ifndef TEMPLATE_RTC_H
#define TEMPLATE_RTC_H

/*!
 * 实例:
 * 1. 初始化（必须先初始化）
 *     Rtc_Init();
 *
 * 2. 设置日期, 如设置为 2021-11-30 星期二
 *     Rtc_SetDate(21, 11, 30, RTC_Weekday_Tuesday);
 *
 * 3. 设置时间【24小时制】，如设置为 13时 14分 15秒
 *     Rtc_SetTime(13, 14, 15);
 */

void Rtc_Init();

void Rtc_SetDate(u8 year, u8 month, u8 date, u8 weekDay);

void Rtc_SetTime(u8 hours, u8 minutes, u8 seconds);

#endif //TEMPLATE_RTC_H

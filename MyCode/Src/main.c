#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "main.h"

/*!
 * Project: STM32F4 GY-906 体温测量
 *
 * 温度保存在 W25Q128 的 0x010000 处，以二进制数据保存 TemperatureData 结构体
 *
 * 为了方便处理数据， TD.T 当成一个 环形数组， TD.offset 记录了 起始位置, TD.count 记录数据数量
 */

#define SAVE_ADDRESS 0x010000
TemperatureData TD;
int offset = 0;
int showed = 0;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // 重定向 printf
    printf_redirect();

    printf("\nStart init\n");

    // 定时器 初始化
    Delay_Init();
    // GY-906 初始化
    Gy906_Init();
    // RTC 初始化
    Rtc_Init();
    // OLed 初始化
    Oled_Init();
    Oled_Fill(0x00);
    // 初始化 TIM3 定时器，显示时间
    Tim3_Init();
    // 初始化 Tim4 定时器，配置中断
    Tim4_Init();
    // 按钮初始化，并开启中断
    Key_Init(KEY_0);
    Key_NVIC_Init(KEY_0, EXTI_Trigger_Rising, 1, 2);
    Key_Init(KEY_1);
    Key_NVIC_Init(KEY_1, EXTI_Trigger_Rising_Falling, 1, 3);
    // 初始化 DHT11
    Dht11_Init();
    Tim2_Init();
    // 初始化 Tim5 定时器，定时启动 ESP8266，获取时间
    Tim5_Init();
    // 初始化 SPI，用于与 Flash 通信
    Spi_Init();
    // 按钮初始化，并开启中断，用于切换显示温度
    Key_Init(KEY_2);
    Key_NVIC_Init(KEY_2, EXTI_Trigger_Rising, 1, 0);
    Key_Init(KEY_3);
    Key_NVIC_Init(KEY_3, EXTI_Trigger_Rising, 1, 0);
    // 蜂鸣器报警
    Beep_Init();
    Tim1_Init();

    printf("Init Finish\n");

    // w25q128 设备ID
    printf("w25q128 ID: %#X\n", W25q128_Id());
    // 读取保存的数据
    W25q128_Read(SAVE_ADDRESS, (u8 *) &TD, sizeof(TD));
    if (Check() == 0) {
        TD.count = 0;
        printf("Null Data or Block Data, Over Write\n");
        bzero((u8 *) &TD, sizeof(TD));
    } else {
        printf("Count: %lu\n", TD.count);
    }
    Save_Data();
    Show_Temperature();

    while (1) {
    }
}

#pragma clang diagnostic pop

void Sleep() {
    // 关闭 TIM3 (定时刷新时间)
    TIM_Cmd(TIM3, DISABLE);
    // 关闭 TIM4 (定时刷新温湿度)
    TIM_Cmd(TIM4, DISABLE);
    // 关闭 TIM5 (定时校准时间)
    TIM_Cmd(TIM5, DISABLE);
    // 关闭 TIM1 (蜂鸣器)
    TIM_Cmd(TIM1, DISABLE);
    // 关闭 OLED
    Oled_OFF();
}

void Weak() {
    Oled_ON();
    TIM_Cmd(TIM3, ENABLE);
//    TIM_Cmd(TIM4, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
}

void Show_Temperature() {
    if (TD.count == 0) {
        return;
    }
    for (int i = 0; i < TD.count; i++) {
        printf("No.%2.2lu, T: %.2f\n", TD.count - i, TD.T[TD.count - i - 1]);
    }
}

void Add_Temperature(float t) {
    if (TD.count == 32) {
        memcpy((u8 *) &(TD.T[0]), (u8 *) &(TD.T[1]), sizeof(float) * 31);
        TD.T[31] = t;
    } else {
        TD.T[TD.count++] = t;
    }
    Save_Data();
}

int Check() {
    if (TD.count > 32) {
        return 0;
    }

    if (TD.check != Build_Hash()) {
        return 0;
    }
    return 1;
}

// 一个校验算法，灵感来源于 simhash
u32 Build_Hash() {
    u8 list[32] = {0};

    for (int i = 0; i < 8; i++) {
        if ((TD.count & (0x00000001 << i))) {
            list[i]++;
        } else {
            list[i]--;
        }
    }

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            if ((*(u32 *) &(TD.T[i])) & (0x00000001 << j)) {
                list[j]++;
            } else {
                list[j]--;
            }
        }
    }

    u32 hash = 0x00000000;
    for (int i = 0; i < sizeof(list); i++) {
        if (list[i] > 0) {
            hash |= (0x00000001 << i);
        }
    }

    return hash;
}

// 覆写数据
void Save_Data() {
    TD.check = Build_Hash();
    W25q128_SectorErase(SAVE_ADDRESS);
    W25q128_Write(SAVE_ADDRESS, (u8 *) &TD, sizeof(TD));
}

void Tim1_Init() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {
            .TIM_Prescaler=168 - 1,
            .TIM_Period=1000 - 1,
            .TIM_CounterMode=TIM_CounterMode_Up,
            .TIM_ClockDivision=TIM_CKD_DIV1,
    };
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=TIM1_UP_TIM10_IRQn,
            .NVIC_IRQChannelPreemptionPriority=2,
            .NVIC_IRQChannelSubPriority=1,
            .NVIC_IRQChannelCmd=ENABLE,
    };
    NVIC_Init(&NVIC_InitStruct);

    TIM_Cmd(TIM1, DISABLE);
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
}

void Esp8266_Init() {
    Init:
    esp8266_init();

    int rt = esp8266_exit_transparent_transmission();
    if (rt) {
        printf("esp8266_exit_transparent_transmission fail\n");
        goto Init;
    }
    printf("esp8266_exit_transparent_transmission success\n");
    Delay_s(1);

    rt = esp8266_reset();
    if (rt) {
        printf("esp8266_reset fail\n");
        goto Init;
    }
    printf("esp8266_reset success\n");
    Delay_s(1);

    rt = esp8266_enable_echo(0);
    if (rt) {
        printf("esp8266_enable_echo(0) fail\n");
        goto Init;
    }
    printf("esp8266_enable_echo(0) success\n");
    Delay_s(1);

    rt = esp8266_connect_ap(WIFI_SSID, WIFI_PASSWORD);
    if (rt) {
        printf("esp8266_connect_ap fail\n");
        goto Init;
    }
    printf("esp8266_connect_ap success\n");
    Delay_s(1);

    rt = esp8266_connect_server("TCP", "192.168.8.102", 10086);
    if (rt) {
        printf("esp8266_connect_server fail\n");
        goto Init;
    }
    printf("esp8266_connect_server success\n");
    Delay_s(1);

    rt = esp8266_entry_transparent_transmission();
    if (rt) {
        printf("esp8266_entry_transparent_transmission fail\n");
        goto Init;
    }
    printf("esp8266_entry_transparent_transmission success\n");
    Delay_s(1);
    g_esp8266_rx_cnt = 0;
    g_esp8266_rx_end = 0;
}

void Tim5_Init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {
            .TIM_Prescaler=8400 - 1,
            .TIM_Period=10000 - 1,
            .TIM_CounterMode=TIM_CounterMode_Up,
            .TIM_ClockDivision=TIM_CKD_DIV1,
    };
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=TIM5_IRQn,
            .NVIC_IRQChannelCmd=ENABLE,
            .NVIC_IRQChannelPreemptionPriority=2,
            .NVIC_IRQChannelSubPriority=0,
    };
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
}

void Tim2_Init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimBaseStruct = {
            .TIM_Prescaler=8400 - 1,
            .TIM_Period=20000 - 1,
            .TIM_CounterMode=TIM_CounterMode_Up,
            .TIM_ClockDivision=TIM_CKD_DIV1,
    };
    TIM_TimeBaseInit(TIM2, &TIM_TimBaseStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=TIM2_IRQn,
            .NVIC_IRQChannelCmd=ENABLE,
            .NVIC_IRQChannelPreemptionPriority=1,
            .NVIC_IRQChannelSubPriority=3,
    };
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void Tim4_Init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimBaseStruct = {
            .TIM_Prescaler=8400 - 1,
            .TIM_Period=10000 - 1,
            .TIM_CounterMode=TIM_CounterMode_Up,
            .TIM_ClockDivision=TIM_CKD_DIV1,
    };
    TIM_TimeBaseInit(TIM4, &TIM_TimBaseStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=TIM4_IRQn,
            .NVIC_IRQChannelCmd=ENABLE,
            .NVIC_IRQChannelPreemptionPriority=1,
            .NVIC_IRQChannelSubPriority=0,
    };
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM4, DISABLE);
}

void Tim3_Init() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimBaseStruct = {
            .TIM_Prescaler=8400 - 1,
            .TIM_Period=10000 - 1,
            .TIM_CounterMode=TIM_CounterMode_Up,
            .TIM_ClockDivision=TIM_CKD_DIV1,
    };
    TIM_TimeBaseInit(TIM3, &TIM_TimBaseStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {
            .NVIC_IRQChannel=TIM3_IRQn,
            .NVIC_IRQChannelCmd=ENABLE,
            .NVIC_IRQChannelPreemptionPriority=1,
            .NVIC_IRQChannelSubPriority=1,
    };
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}

__attribute__((unused)) void TIM1_UP_TIM10_IRQHandler() {
    static int count = 0;
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) {
        if (count >= 1000) {
            count = 0;
            TIM_Cmd(TIM1, DISABLE);
            Beep_Off();
        } else if (count % 100 == 0) {
            Beep_Toggle();
            count++;
        } else {
            count++;
        }

        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}

__attribute__((unused)) void EXTI4_IRQHandler() {
    if (EXTI_GetITStatus(EXTI_Line4) == SET) {
        for (volatile int i = 0; i < 100; i++) {
            for (volatile int j = 0; j < 100; j++) {}
        }

        if (PEin(3) && TD.count != 0) {
            // 下一条温度记录
            if (offset < 0) {
                offset = (int) TD.count + offset + 3;
            }
            if (showed == 0) {
                offset = (int) TD.count;
                showed = 1;
            } else if (offset - (int) TD.count == 1 || offset == 0) {
                offset = 1;
            }
            Oled_ShowTemperature_24x48(TD.T[offset - 1], offset, (int) TD.count);
            offset++;
        }

        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

__attribute__((unused)) void EXTI3_IRQHandler() {
    if (EXTI_GetITStatus(EXTI_Line3) == SET) {
        for (volatile int i = 0; i < 100; i++) {
            for (volatile int j = 0; j < 100; j++) {}
        }

        if (PEin(3) && TD.count != 0) {
            if (offset > 0) {
                offset = offset - (int) TD.count - 3;
            }
            if (showed == 0) {
                offset = -2;
                showed = 1;
            } else if (offset + (int) TD.count == -1) {
                offset = -1;
            } else if (offset == 0) {
                offset = -2;
            }
            // 上一条温度记录
            Oled_ShowTemperature_24x48(TD.T[TD.count + offset], (int) TD.count + offset + 1, (int) TD.count);
            offset--;
        }

        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

__attribute__((unused)) void TIM5_IRQHandler() {
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET) {
        static int count = 0;


        if (count == 60) {
            count = 0;
            TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
            Esp8266_Init();
            u8 t;
            esp8266_send_bytes(&t, 1);
        } else {
            count++;
            TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        }

    }
}

__attribute__((unused)) void TIM2_IRQHandler() {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
        int ret = Dht11_Start();
        if (ret != 0) {
            printf("Start Dht11 Failure: %d\n", ret);
        } else {
            u8 buf[5] = {0};
            for (int i = 0; i < 5; i++) {
                buf[i] = Dht11_Read_Byte();
            }

            Oled_ShowEnvRHTA(buf[0], buf[2]);
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

__attribute__((unused)) void TIM4_IRQHandler() {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) {
        u8 buf[3] = {0};
        Gy906_Read(0x07, buf, 3);
        float T = ((float) *(u16 *) buf) * 0.02 - 273.15 + 3; // NOLINT(cppcoreguidelines-narrowing-conversions)

        Oled_ShowTemperature_24x48(T, (int) TD.count, (int) TD.count);
        printf("Auto temperature: %.2f\n", T);

        if (T >= 37.3) {
            TIM_Cmd(TIM1, ENABLE);
        }

        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        offset = 0;
    }
}

__attribute__((unused)) void EXTI2_IRQHandler() {
    static int state = 0;
    static int is_sleep = 0;
    static RTC_TimeTypeDef time;
    static int IN = 0;

    if (EXTI_GetITStatus(EXTI_Line2) == SET) {
        IN = PEin(2);
        for (volatile int i = 0; i < 100; i++) {
            for (volatile int j = 0; j < 100; j++) {}
        }

        if (IN == PEin(2)) {
            if (!IN) {
                RTC_GetTime(RTC_Format_BIN, &time);
            } else {
                RTC_TimeTypeDef t;
                RTC_GetTime(RTC_Format_BIN, &t);
                if (t.RTC_Hours > time.RTC_Hours || t.RTC_Minutes > time.RTC_Minutes ||
                    (t.RTC_Seconds - time.RTC_Seconds >= 2)) {
                    state = 0;
                    if (is_sleep) {
                        printf("Weak\n");
                        is_sleep = 0;
                        Weak();
                    } else {
                        printf("Sleep\n");
                        is_sleep = 1;
                        Sleep();
                    }
                } else if (!is_sleep) {
                    if (state) {
                        TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
                        TIM_Cmd(TIM4, DISABLE);
                    } else {
                        TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
                        TIM_Cmd(TIM4, ENABLE);
                    }
                    state = !state;
                }
            }
        }
    }

    EXTI_ClearITPendingBit(EXTI_Line2);
}

__attribute__((unused)) void EXTI0_IRQHandler() {
    if (EXTI_GetITStatus(EXTI_Line0) == SET) {
        for (volatile int i = 0; i < 100; i++) {
            for (volatile int j = 0; j < 100; j++) {}
        }
        if (PAin(0)) {
            u8 buf[3] = {0};
            Gy906_Read(0x07, buf, 3);
            float T = ((float) *(u16 *) buf) * 0.02 - 273.15 + 3; // NOLINT(cppcoreguidelines-narrowing-conversions)

            printf("Temperature: %.2f\n", T);

            Add_Temperature(T);
            Save_Data();
            offset = 0;

            Oled_ShowTemperature_24x48(T, (int) TD.count, (int) TD.count);
            if (T >= 37.3) {
                TIM_Cmd(TIM1, ENABLE);
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

__attribute__((unused)) void TIM3_IRQHandler() {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
        RTC_TimeTypeDef RTC_TimeStruct;
        RTC_DateTypeDef RTC_DateStruct;
        RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
        RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
        char buf[255] = {0};
        sprintf(buf, "20%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month,
                RTC_DateStruct.RTC_Date, RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes,
                RTC_TimeStruct.RTC_Seconds);
        Oled_ShowAscii(7, 0, buf, s6x8);

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

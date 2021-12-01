#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "main.h"

/*!
 * Project: STM32F4 GY-906 体温测量
 * Version: 0.0.3
 */

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
    Key_NVIC_Init(KEY_1, EXTI_Trigger_Rising, 1, 3);
    // 初始化 DHT11
    Dht11_Init();
    Tim2_Init();
    // 初始化 Tim5 定时器，定时启动 ESP8266，获取时间
    Tim5_Init();

    printf("Init Finish\n");

    while (1) {
    }
}

#pragma clang diagnostic pop

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

    rt = esp8266_connect_server("TCP", "192.168.5.132", 10086);
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
            .TIM_Period=5000 - 1,
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
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
        u8 buf[3] = {0};
        Gy906_Read(0x07, buf, 3);
        float T = ((float) *(u16 *) buf) * 0.02 - 273.15; // NOLINT(cppcoreguidelines-narrowing-conversions)

        Oled_ShowTemperature_24x48(T);
        printf("Auto temperature: %.2f\n", T);

        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}

__attribute__((unused)) void EXTI2_IRQHandler() {
    static int state = 0;

    if (EXTI_GetITStatus(EXTI_Line2) == SET) {
        for (volatile int i = 0; i < 100; i++) {
            for (volatile int j = 0; j < 100; j++) {}
        }

        if (PEin(2)) {
            if (state) {
                TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
                TIM_Cmd(TIM4, DISABLE);
            } else {
                TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
                TIM_Cmd(TIM4, ENABLE);
            }
            state = !state;
        }

        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

__attribute__((unused)) void EXTI0_IRQHandler() {
    if (EXTI_GetITStatus(EXTI_Line0) == SET) {
        for (volatile int i = 0; i < 100; i++) {
            for (volatile int j = 0; j < 100; j++) {}
        }
        if (PAin(0)) {
            u8 buf[3] = {0};
            Gy906_Read(0x07, buf, 3);
            float T = ((float) *(u16 *) buf) * 0.02 - 273.15; // NOLINT(cppcoreguidelines-narrowing-conversions)

            Oled_ShowTemperature_24x48(T);

            printf("Temperature: %.2f\n", T);
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

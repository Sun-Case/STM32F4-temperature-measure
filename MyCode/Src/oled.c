#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "codetab.h"

/*!
 * OLED 12864 显示屏
 * 尺寸: 128 * 64
 * 通信协议: IIC
 * 使用引脚:
 *     GPIO D 1 : VCC
 *     GPIO D 15: GND
 *     GPIO E 8 : SCK(SCL)
 *     GPIO E 10: SDA
 */

// 宏定义
#define delay_us(delay) Delay_us(delay)
#define delay_ms(delay) Delay_ms(delay)
#define SDA_Out PEout(10)
#define SDA_In PEin(10)
#define SCL_Out PEout(8)
#define OLED_Address 0x78

// SDA 引脚模式切换
static void oled_ToggleMode(GPIOMode_TypeDef mode) {
    static GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_Speed=GPIO_High_Speed,
            .GPIO_Pin=GPIO_Pin_10,
    };
    GPIO_InitStruct.GPIO_Mode = mode;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
}

// 起始信号
static void oled_Start() {
    oled_ToggleMode(GPIO_Mode_OUT);

    SDA_Out = 1;
    SCL_Out = 1;
    delay_us(1);

    SDA_Out = 0;
    delay_us(1);

    SCL_Out = 0;
    delay_us(1);
}

// 停止信号
static void oled_Stop() {
    oled_ToggleMode(GPIO_Mode_OUT);

    SDA_Out = 0;
    SCL_Out = 0;
    delay_us(1);

    SCL_Out = 1;
    delay_us(1);

    SDA_Out = 1;
    delay_us(1);
}

// 发送1字节
static void oled_SendByte(u8 byte) {
    oled_ToggleMode(GPIO_Mode_OUT);

    for (int i = 7; i >= 0; i--) {
        if (byte & (0x01 << i)) {
            SDA_Out = 1;
        } else {
            SDA_Out = 0;
        }
        delay_us(1);

        SCL_Out = 1;
        delay_us(1);

        SCL_Out = 0;
        delay_us(1);
    }
}

// 接收 Ack
static u8 oled_RecvAck() {
    oled_ToggleMode(GPIO_Mode_IN);

    SCL_Out = 1;
    delay_us(1);

    u8 ack = (SDA_In != 0);
    SCL_Out = 0;
    delay_us(1);

    return ack;
}

// 接收1字节
static u8 oled_RecvByte() {
    oled_ToggleMode(GPIO_Mode_IN);

    u8 data = 0x00;

    for (int i = 7; i >= 0; i--) {
        SCL_Out = 1;
        delay_us(1);

        if (SDA_In) {
            data |= (0x01 << i);
        }
        SCL_Out = 0;
        delay_us(1);
    }

    return data;
}

// 写OLED设备
void oled_Write(u8 address, u8 *buf, u8 len) {
    oled_Start();

    oled_SendByte(OLED_Address);
    if (oled_RecvAck() != 0) {
        printf("OLED Write: Send Device Address Failure\n");
        goto Label;
    }

    oled_SendByte(address);
    if (oled_RecvAck() != 0) {
        printf("OLED Write: Send Command Failure\n");
        goto Label;
    }

    for (int i = 0; i < len; i++) {
        oled_SendByte(buf[i]);
        if (oled_RecvAck() != 0) {
            printf("OLED Write: Send Data Failure\n");
            goto Label;
        }
    }

    Label:
    oled_Stop();
}

// 发送OLED命令
void WriteCmd(u8 oled_command) {
    oled_Write(0x00, &oled_command, 1);
}

// 发送OLED数据
void WriteDat(u8 oled_data) {
    oled_Write(0x40, &oled_data, 1);
}

// 初始化
void Oled_Init() {
    // 初始化引脚
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    // (GPIO D 1) & (GPIO D 15)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {
                .GPIO_Mode=GPIO_Mode_OUT,
                .GPIO_Pin=GPIO_Pin_1 | GPIO_Pin_15,
                .GPIO_OType=GPIO_OType_PP,
                .GPIO_PuPd=GPIO_PuPd_UP,
                .GPIO_Speed=GPIO_Medium_Speed,
        };
        GPIO_Init(GPIOD, &GPIO_InitStruct);

        PDout(1) = 1;
        PDout(15) = 0;
    }

    // (GPIO E 8) & (GPIO E 10)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {
                .GPIO_Speed=GPIO_High_Speed,
                .GPIO_PuPd=GPIO_PuPd_UP,
                .GPIO_OType=GPIO_OType_PP,
                .GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_10,
                .GPIO_Mode=GPIO_Mode_OUT,
        };
        GPIO_Init(GPIOE, &GPIO_InitStruct);

        PEout(8) = 1;
        PEout(10) = 1;
    }

    // 初始化 OLED 设备
    delay_ms(100);

    WriteCmd(0xAE); //display off
    WriteCmd(0x20);    //Set Memory Addressing Mode
    WriteCmd(
            0x10);    //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    WriteCmd(0xb0);    //Set Page Start Address for Page Addressing Mode,0-7
    WriteCmd(0xc8);    //Set COM Output Scan Direction
    WriteCmd(0x00); //---set low column address
    WriteCmd(0x10); //---set high column address
    WriteCmd(0x40); //--set start line address
    WriteCmd(0x81); //--set contrast control register
    WriteCmd(0xff); //亮度调节 0x00~0xff
    WriteCmd(0xa1); //--set segment re-map 0 to 127
    WriteCmd(0xa6); //--set normal display
    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
    WriteCmd(0x3F); //
    WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    WriteCmd(0xd3); //-set display offset
    WriteCmd(0x00); //-not offset
    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
    WriteCmd(0xf0); //--set divide ratio
    WriteCmd(0xd9); //--set pre-charge period
    WriteCmd(0x22); //
    WriteCmd(0xda); //--set com pins hardware configuration
    WriteCmd(0x12);
    WriteCmd(0xdb); //--set vcomh
    WriteCmd(0x20); //0x20,0.77xVcc
    WriteCmd(0x8d); //--set DC-DC enable
    WriteCmd(0x14); //
    WriteCmd(0xaf); //--turn on oled panel
}

// 设置起始点坐标
void Oled_SetPos(u8 x, u8 y) {
    WriteCmd(0xb0 + y);
    WriteCmd(((x & 0xf0) >> 4) | 0x10);
    WriteCmd((x & 0x0f) | 0x01);
}

// 全屏填充
void Oled_Fill(u8 fill_Data) {
    unsigned char m, n;
    for (m = 0; m < 8; m++) {
        WriteCmd(0xb0 + m);        //page0-page1
        WriteCmd(0x00);        //low column start address
        WriteCmd(0x10);        //high column start address
        for (n = 0; n < 128; n++) {
            WriteDat(fill_Data);
        }
    }
}

// 清屏
void Oled_CLS(void) {
    Oled_Fill(0x00);
}

// 将 OLED 从休眠中唤醒
void Oled_ON(void) {
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}

// 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
void Oled_OFF(void) {
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X10);  //关闭电荷泵
    WriteCmd(0XAE);  //OLED休眠
}

void Oled_ShowAscii(u8 x, u8 y, char ascii[], FontSize fs) {
    u8 off = 0;
    if (fs == s6x8) {
        while (ascii[off] != '\0') {
            if (x > 122) {
                x = 0;
                y++;
            }

            Oled_SetPos(x, y);

            for (int i = 0; i < 6; i++) {
                WriteDat(Ascii_6x8[ascii[off]][i]);
            }

            x += 6;
            off++;
        }
    } else if (fs == s8x16) {
        while (ascii[off] != '\0') {
            if (x > 120) {
                x = 0;
                y += 2;
            }

            Oled_SetPos(x, y);

            for (int i = 0; i < 8; i++) {
                WriteDat(Ascii_8x16[ascii[off]][i]);
            }

            Oled_SetPos(x, y + 1);
            for (int i = 0; i < 8; i++) {
                WriteDat(Ascii_8x16[ascii[off]][i + 8]);
            }

            x += 8;
            off++;
        }
    }
}

void Oled_Show_24x48(u8 x, u8 y, char ch[]) {
    u8 off = 0;
    while (ch[off] != '\0') {
        int index;
        switch (ch[off]) {
            case '0' ... '9':
                index = ch[off] - '0';
                break;
            case '.':
                index = 10;
                break;
            case 'L':
                index = 11;
                break;
            case 'o':
                index = 12;
                break;
            case 'H':
                index = 13;
                break;
            case 'i':
                index = 14;
                break;
            default:
                off++;
                continue;
        }

        if (x > 104) {
            x = 0;
            y += 6;
        }

        for (int i = 0; i < 8; i++) {
            Oled_SetPos(x, y + i);
            for (int j = 0; j < 24; j++) {
                WriteDat(Number_28x56[index][i][j]);
            }
        }

        x += 24;
        if (ch[off] == '.') {
            x -= 12;
        }
        off++;
    }
}

void Oled_ShowTemperature_24x48(float T) {
    // 清屏
    for (int i = 0; i < 8; i++) {
        Oled_SetPos(0, 1 + i);
        for (int j = 0; j < 128; j++) {
            WriteDat(0x00);
        }
    }

    if (T < 25) {
        // Too Low
        Oled_Show_24x48(40, 1, "Lo");
    } else if (T > 45) {
        // Too High
        Oled_Show_24x48(52, 1, "H");
    } else {
        // 格式化为 XX.XX ℃
        char buf[6] = {0};
        sprintf(buf, "%5.1f", T);
        Oled_Show_24x48(14, 1, buf);

        for (int i = 0; i < 2; i++) {
            Oled_SetPos(100, 4 + i);
            for (int j = 0; j < 16; j++) {
                WriteDat(unit[i][j]);
            }
        }
    }
}

#undef delay_us

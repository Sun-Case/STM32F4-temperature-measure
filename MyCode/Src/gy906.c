#include <stdio.h>
#include "stm32f4xx.h"
#include "sys.h"
#include "gy906.h"
#include "delay.h"

/*!
 * GY-906 设备地址: 0x00H 或 0x5AH
 * GY-906 使用 SMBus 协议，与 IIC协议 类似
 * 大部分基于 IIC协议规范，但又有区别，所以使用软件模拟 SMBus
 */

/*!
 * 使用引脚如下
 *
 * SDA: GPIO B - 9
 * SCL: GPIO B - 8
 */

// 宏定义
#define SDA_Out PBout(9)
#define SDA_In PBin(9)
#define SCL_Out PBout(8)
#define GY906_Address 0x00 // 或 0x5AH
// 延时函数
#define delay_us Delay_us(5)

// 初始化引脚
void Gy906_Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Mode=GPIO_Mode_OUT,
            .GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_9,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_Speed=GPIO_High_Speed,
    };
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    SDA_Out = 1;
    SCL_Out = 1;
}

// 引脚模式切换
void Gy906_ToggleGpioMode(GPIOMode_TypeDef mode) {
    static GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Speed=GPIO_High_Speed,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_Pin=GPIO_Pin_9,
    };
    GPIO_InitStruct.GPIO_Mode = mode;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// 启动信号
void Gy906_Start() {
    Gy906_ToggleGpioMode(GPIO_Mode_OUT);

    SDA_Out = 1;
    SCL_Out = 1;
    delay_us;

    SDA_Out = 0;
    delay_us;

    SCL_Out = 0;
    delay_us;
}

// 停止信号
void Gy906_Stop() {
    Gy906_ToggleGpioMode(GPIO_Mode_OUT);

    SDA_Out = 0;
    SCL_Out = 0;
    delay_us;

    SCL_Out = 1;
    delay_us;

    SDA_Out = 1;
    delay_us;
}

// 发送1字节
void Gy906_SendByte(u8 byte) {
    Gy906_ToggleGpioMode(GPIO_Mode_OUT);

    for (int i = 7; i >= 0; i--) {
        if (byte & (0x01 << i)) {
            SDA_Out = 1;
        } else {
            SDA_Out = 0;
        }
        delay_us;

        SCL_Out = 1;
        delay_us;

        SCL_Out = 0;
        delay_us;
    }
}

// 接收 ACK
u8 Gy906_RecvAck() {
    Gy906_ToggleGpioMode(GPIO_Mode_IN);

    SCL_Out = 1;
    delay_us;

    u8 ack = (SDA_In != 0);
    SCL_Out = 0;
    delay_us;

    return ack;
}

// 接收1字节
u8 Gy906_RecvByte() {
    u8 data = 0x00;

    Gy906_ToggleGpioMode(GPIO_Mode_IN);

    for (int i = 7; i >= 0; i--) {
        SCL_Out = 1;
        delay_us;

        if (SDA_In) {
            data |= (0x01 << i);
        }
        SCL_Out = 0;
        delay_us;
    }

    return data;
}

// 发送 ACK
void Gy906_SendAck(u8 bit) {
    Gy906_ToggleGpioMode(GPIO_Mode_OUT);

    if (bit) {
        SDA_Out = 1;
    } else {
        SDA_Out = 0;
    }
    delay_us;

    SCL_Out = 1;
    delay_us;

    SCL_Out = 0;
    delay_us;
}

/*!
 * 读取 GY-906 数据
 * @param address 寄存器地址
 * @param buf 读取后的数据存储在buf
 * @param len 要读取的字节数
 */
void Gy906_Read(u8 address, u8 *buf, u8 len) {
    Gy906_Start();

    Gy906_SendByte(GY906_Address);
    if (Gy906_RecvAck() != 0) {
        printf("GY-906 Read: Send Device Address Failure\n");
        goto Label;
    }

    Gy906_SendByte(address);
    if (Gy906_RecvAck() != 0) {
        printf("GY-906 Read: Send Command Failure\n");
        goto Label;
    }

    Gy906_Start();

    Gy906_SendByte(GY906_Address | 0x01);
    if (Gy906_RecvAck() != 0) {
        printf("GY-906 Read: Send ReadBit Failure\n");
        goto Label;
    }

    for (u8 i = 0; i < len; i++) {
        buf[i] = Gy906_RecvByte();
        Gy906_SendAck(0);
    }
    Gy906_SendAck(1);

    Label:
    Gy906_Stop();
}

#undef delay

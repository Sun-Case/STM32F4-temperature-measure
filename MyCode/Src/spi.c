#include "stm32f4xx.h"
#include "sys.h"
#include "spi.h"

void Spi_Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {
            .GPIO_Mode=GPIO_Mode_AF,
            .GPIO_OType=GPIO_OType_PP,
            .GPIO_Pin=GPIO_Pin_4 | GPIO_Pin_5,
            .GPIO_PuPd=GPIO_PuPd_UP,
            .GPIO_Speed=GPIO_Speed_50MHz
    };
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    PBout(14) = 1;

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

    SPI_InitTypeDef SPI_InitStruct = {
            .SPI_Direction=SPI_Direction_2Lines_FullDuplex,
            .SPI_Mode=SPI_Mode_Master,
            .SPI_DataSize=SPI_DataSize_8b,
            .SPI_CPHA=SPI_CPHA_1Edge,
            .SPI_CPOL=SPI_CPOL_Low,
            .SPI_FirstBit=SPI_FirstBit_MSB,
            .SPI_NSS=SPI_NSS_Soft,
            .SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_16,
            .SPI_CRCPolynomial=7
    };
    SPI_Init(SPI1, &SPI_InitStruct);

    SPI_Cmd(SPI1, ENABLE);
}

u8 Spi_Recv_Send_Byte(u8 tx_data) {
    u8 rx_data = 0x00;

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    SPI_I2S_SendData(SPI1, tx_data);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    rx_data = SPI_I2S_ReceiveData(SPI1);

    return rx_data;
}

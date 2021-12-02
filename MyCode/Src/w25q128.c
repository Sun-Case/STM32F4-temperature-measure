#include "stm32f4xx.h"
#include "sys.h"
#include "spi.h"
#include "w25q128.h"

#define F_CS PBout(14)

u16 W25q128_Id() {
    F_CS = 0;

    Spi_Recv_Send_Byte(0x90);

    Spi_Recv_Send_Byte(0x00);
    Spi_Recv_Send_Byte(0x00);
    Spi_Recv_Send_Byte(0x00);

    u16 id = 0x0000;
    id |= ((u16) Spi_Recv_Send_Byte(0x00)) << 8;
    id |= ((u16) Spi_Recv_Send_Byte(0x00));

    F_CS = 1;

    return id;
}

void W25q128_WriteEnable() {
    F_CS = 0;

    Spi_Recv_Send_Byte(0x06);

    F_CS = 1;
}

void W25q128_WaitBusy() {
    F_CS = 0;

    Spi_Recv_Send_Byte(0x05);

    while ((Spi_Recv_Send_Byte(0x00) & 0x01) != 0);

    F_CS = 1;
}

void W25q128_SectorErase(u32 address) {
    W25q128_WriteEnable();

    F_CS = 0;

    Spi_Recv_Send_Byte(0x20);

    Spi_Recv_Send_Byte((address >> 16) & 0xFF);
    Spi_Recv_Send_Byte((address >> 8) & 0xFF);
    Spi_Recv_Send_Byte((address >> 0) & 0xFF);

    F_CS = 1;

    W25q128_WaitBusy();
}

void W25q128_Write(u32 address, u8 *buf, u8 len) {
    W25q128_WriteEnable();

    F_CS = 0;

    Spi_Recv_Send_Byte(0x02);

    Spi_Recv_Send_Byte((address >> 16) & 0xFF);
    Spi_Recv_Send_Byte((address >> 8) & 0xFF);
    Spi_Recv_Send_Byte((address >> 0) & 0xFF);

    for (int i = 0; i < len; i++) {
        Spi_Recv_Send_Byte(buf[i]);
    }

    F_CS = 1;

    W25q128_WaitBusy();
}

void W25q128_Read(u32 address, u8 *buf, u8 len) {
    W25q128_WaitBusy();

    F_CS = 0;

    Spi_Recv_Send_Byte(0x03);

    Spi_Recv_Send_Byte((address >> 16) & 0xFF);
    Spi_Recv_Send_Byte((address >> 8) & 0xFF);
    Spi_Recv_Send_Byte((address >> 0) & 0xFF);

    for (int i = 0; i < len; i++) {
        buf[i] = Spi_Recv_Send_Byte(0x00);
    }

    F_CS = 1;
}

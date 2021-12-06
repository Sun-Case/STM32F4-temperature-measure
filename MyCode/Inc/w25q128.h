#ifndef TEMPLATE_W25Q128_H
#define TEMPLATE_W25Q128_H

#include "stm32f4xx.h"

u16 W25q128_Id();

void W25q128_SectorErase(u32 address);

void W25q128_Write(u32 address, u8 *buf, int len);

void W25q128_Read(u32 address, u8 *buf, int len);

#endif //TEMPLATE_W25Q128_H

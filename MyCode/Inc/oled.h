#ifndef TEMPLATE_OLED_H
#define TEMPLATE_OLED_H

typedef enum {
    s6x8 = 1,
    s8x16,
} FontSize;

void Oled_Init();

void Oled_Fill(u8 fill_Data);

void Oled_CLS(void);

void Oled_ON(void);

void Oled_OFF(void);

void Oled_ShowAscii(u8 x, u8 y, char ascii[], FontSize fs);

void Oled_ShowTemperature_24x48(float T, int index, int count);

void Oled_ShowEnvRHTA(u8 rh, u8 ta);

#endif //TEMPLATE_OLED_H

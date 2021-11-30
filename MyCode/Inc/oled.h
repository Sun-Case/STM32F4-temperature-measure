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

//void Oled_ShowStr(u8 x, u8 y, u8 ch[], u8 TextSize);
//
//void Oled_ShowCN(u8 x, u8 y, u8 N);
//
//void Oled_DrawBMP(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[]);

void Oled_ShowAscii(u8 x, u8 y, char ascii[], FontSize fs);

void Oled_ShowTemperature_24x48(float T);

#endif //TEMPLATE_OLED_H

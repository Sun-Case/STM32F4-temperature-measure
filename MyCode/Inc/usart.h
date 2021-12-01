#ifndef TEMPLATE_USART_H
#define TEMPLATE_USART_H

#include "stm32f4xx.h"

void printf_redirect();

void usart3_init(uint32_t baud);

void usart3_send_str(char *str);

void usart3_send_bytes(uint8_t *buf, uint32_t len);

#endif //TEMPLATE_USART_H

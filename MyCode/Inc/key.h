#ifndef TEMPLATE_KEY_H
#define TEMPLATE_KEY_H

typedef enum {
    KEY_0 = 1,
    KEY_1,
    KEY_2,
    KEY_3
} KEY_TypeDef;

void Key_Init(KEY_TypeDef keyX);

void Key_NVIC_Init(KEY_TypeDef keyX, EXTITrigger_TypeDef triggerType, u8 preemptionPriority, u8 subPriority);

#endif //TEMPLATE_KEY_H

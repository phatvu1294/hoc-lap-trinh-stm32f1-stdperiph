#ifndef _DELAY_H
#define _DELAY_H

#include <stm32f10x.h>

/* Nguyên mẫu hàm public */
void delay_init(void);
uint32_t millis(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif

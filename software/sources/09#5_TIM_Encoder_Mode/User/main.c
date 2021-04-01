#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\usart1.h"

uint16_t counter = 0;
uint16_t counterOld = 0;
uint8_t direction = 0;
char strBuffer[32];

int main(void)
{
  /* ___LIB___ */
	
	/* Khởi tạo thư viện usart1 */
	usart1_init();
	
	/* ___GPIO___ */
	
	/* Bật Clock PortA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/* Cấu hình PA6, PA7 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* ___TIM___ */
	
	/* Bật Clock TIM3 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	/* Cấu hình TIM3 */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65535;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 3;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	TIM_ARRPreloadConfig(TIM3, DISABLE);
	
	/* Cấu hình TIM3 Encoder Mode */
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);
	
	/* Cấu hình TIM3 Input Capture */
	TIM_ICInitTypeDef TIM_ICInitStucture;
	TIM_ICInitStucture.TIM_ICFilter = 4;
	TIM_ICInit(TIM3, &TIM_ICInitStucture);
  
  /* Bật TIM3 */
  TIM_Cmd(TIM3, ENABLE);

  while (1)
  {
    /* Đọc giá trị của Encoder */
		counter = TIM_GetCounter(TIM3);
		
		/* Đọc hướng của Encoder */
		direction = ((TIM3->CR1 & TIM_CR1_DIR) == TIM_CR1_DIR);
		
		/* Nếu giá trị thay đổi */
		if (counter != counterOld)
		{
			/* Gửi giá trị ra usart1 */
			sprintf(strBuffer, "Counter=%d, Direction=%d\r\n", counter, direction);
			usart1_putString((uint8_t *)strBuffer);
      
      /* Lưu lại giá trị cũ */
      counterOld = counter;
		}
  }
}

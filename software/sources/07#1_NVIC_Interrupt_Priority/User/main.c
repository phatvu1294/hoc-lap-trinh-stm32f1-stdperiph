#include <stm32f10x.h>
#include "..\libraries\delay.h"

/* 
Mức độ ưu tiên ngắt
- Ưu tiên pre-emption:
	- Ngắt mức độ ưu tiên cao hơn có thể làm gián đoạn ngắt có mức độ ưu tiên
	thấp hơn
	- 0 có mức độ ưu tiên pre-emption là cao nhất
- Ưu tiên sub:
	- Nếu có hai hoặc nhiều ngắt đang chờ với cùng mức ưu tiên pre-emption, thì
	mức ưu tiên sub được sử dụng để xác định ngắt nào phải được phục vụ trước
	- 0 có mức độ ưu tiên sub là cao nhất
*/

void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);

int main(void)
{
	/* ___LIB___ */
	
	/* Khởi tạo thư viện delay */
	delay_init();
	
	/* ___GPIO___ */
	
	/* Bật Clock PortA và PortB */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Cấu hình PA0, PA1, PA2 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Cấu hình PB5, PB6, PB7 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ___AFIO___ */
	
	/* Bật Clock AFIO */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* Kết nối Port với ngoại vi ngắt */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	
	/* ___EXTI___ */
	
	/* Cấu hình ngắt EXTI line0 */
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Cấu hình ngắt EXTI line1 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Cấu hình ngắt EXTI line2 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_Init(&EXTI_InitStructure);
	
	/* ___NVIC___ */
  
  /* Cấu hình nhóm ngắt toàn cục */
  /* ((uint32_t)0x00000007) 0 bit  for pre-emption priority, 4 bits for subpriority */
  /* ((uint32_t)0x00000006) 1 bit  for pre-emption priority, 3 bits for subpriority */
  /* ((uint32_t)0x00000005) 2 bits for pre-emption priority, 2 bits for subpriority */
  /* ((uint32_t)0x00000004) 3 bits for pre-emption priority, 1 bit  for subpriority */
  /* ((uint32_t)0x00000003) 4 bits for pre-emption priority, 0 bit  for subpriority */
  NVIC_SetPriorityGrouping((uint32_t)0x00000003); 
	
	/* Cấu hình ngắt toàn cục */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);

  while (1)
  {
		
  }
}

/* Trình phục vụ ngắt EXTI line0 */
void EXTI0_IRQHandler(void)
{
	/* Nếu là cờ ngắt line0 */
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		/* Đọc bit thứ 0 của thanh ghi GPIOA_IDR */
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
    {
			for (uint32_t i = 0; i < 10; i++)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_5);
				delay_ms(200);
				GPIO_ResetBits(GPIOB, GPIO_Pin_5);
				delay_ms(200);
			}
		}
		
		/* Xoá cờ ngắt line0 */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

/* Trình phục vụ ngắt EXTI line1 */
void EXTI1_IRQHandler(void)
{
	/* Nếu là cờ ngắt line1 */
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		/* Đọc bit thứ 1 của thanh ghi GPIOA_IDR */
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_RESET)
    {
			for (uint32_t i = 0; i < 10; i++)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_6);
				delay_ms(200);
				GPIO_ResetBits(GPIOB, GPIO_Pin_6);
				delay_ms(200);
			}
		}
		
		/* Xoá cờ ngắt line1 */
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

/* Trình phục vụ ngắt EXTI line2 */
void EXTI2_IRQHandler(void)
{
	/* Nếu là cờ ngắt line2 */
	if (EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		/* Đọc bit thứ 2 của thanh ghi GPIOA_IDR */
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == Bit_RESET)
    {
			for (uint32_t i = 0; i < 10; i++)
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_7);
				delay_ms(200);
				GPIO_ResetBits(GPIOB, GPIO_Pin_7);
				delay_ms(200);
			}
		}
		
		/* Xoá cờ ngắt line2 */
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

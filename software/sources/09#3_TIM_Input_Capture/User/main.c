#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

uint32_t icValue1 = 0;
uint32_t icValue2 = 0;
uint32_t diffCapture = 0;
uint16_t captureIndex = 0;
double frequency = 0.0;
char strBuffer[32];

void TIM3_IRQHandler(void);

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện delay */
  delay_init();

  /* Khởi tạo thư viện usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortB và PortA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Cấu hình PB6 (TIM4 PWM Channel 1) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternet-function push-pull
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Cấu hình PA6 (TIM3 Input Capture Channel 1) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Input floating
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ___TIM___ */

  /* Bật Clock TIM4 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* Cấu hình TIM4 */
  /* f_clock_source/f_update = f_update * Prescaler * (Counter_Period + 1) */
  /* có f_clock_source = 72MHz */
  /* Với f_update = 1kHz => 72000 = Prescaler * (Counter_Period + 1) */
  /* ta có Counter_Period (max) = 2^16 - 1 = 65535 => Prescaler (min) = 1.099 */
  /* Chọn Prescaler = 72 => Counter_Period = 1000-1 */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;
  TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1; // Timer Clock
  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
  TIM_Cmd(TIM4, ENABLE);

  /* Cấu hình TIM4 PWM Chanel 1 */
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = 500;
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  TIM_OC1FastConfig(TIM4, TIM_OCFast_Enable); // Fast PWM

  /* Bật Clock TIM3 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Cấu hình TIM3 */
  /* f_clock_source = f_input_signal * Prescaler * diff */
  /* có f_clock_source = 72MHz */
  /* Để có được f_input_signal = 1kHz thì: 72000 = Prescaler * diff */
  /* ta có diff (max) = 2^16 - 1 = 65535 => Prescaler (min) = 1.099 */
  /* Chọn Prescaler = 2 */
  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStructure.TIM_Period = 65535; // Nên để lớn nhất
  TIM_TimeBaseInitStructure.TIM_Prescaler = 2 - 1; // Timer Clock
  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
  TIM_Cmd(TIM3, ENABLE);

  /* Cấu hình TIM3 IC Channel 1 */
  TIM_ICInitTypeDef TIM_ICInitStructure;
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; // Timer Clock
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);

  /* Cho phép ngắt TIM3 IC Capture/Compare Channel 1 */
  TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

  /* ___NVIC___ */

  /* Cấu hình ngắt TIM3 toàn cục */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  while (1)
  {
    /* Gửi giá trị tần số ra usart1 */
    sprintf(strBuffer, "Frequency=%.2fHz\r\n", frequency);
    usart1_putString((uint8_t *)strBuffer);
    delay_ms(1000);
  }
}

/* Trình phục vụ ngắt TIM3 */
void TIM3_IRQHandler(void)
{
  /* Nếu cờ CC1IF = 1 và TIM_IT_CC1 được cho phép */
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
  {
    if (captureIndex == 0)
    {
      /* Đọc giá trị Capture cũ */
      icValue1 = TIM_GetCapture1(TIM3);
      captureIndex = 1;
    }
    else if (captureIndex == 1)
    {
      /* Đọc giá trị Capture mới */
      icValue2 = TIM_GetCapture1(TIM3);

      /* Kiểm tra 2 giá trị Capture */
      if (icValue2 > icValue1)
      {
        diffCapture = icValue2 - icValue1;
      }
      else if (icValue2 < icValue1)
      {
        diffCapture = ((65535 - icValue1) + icValue2) + 1;
      }

      /* f_input_signal = f_clock_source / prescaler / diff */
      frequency = 72000000.0 / 2.0 / diffCapture;
      captureIndex = 0;
    }

    /* Xoá cờ CC1IF */
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
  }
}

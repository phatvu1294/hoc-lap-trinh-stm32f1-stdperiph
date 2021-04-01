#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

uint32_t icValue1 = 0;
uint32_t icValue2 = 0;
double dutyCycle = 0.0;
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

  /* ____GPIO____ */

  /* Bật Clock PortB và PortA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Cấu hình PB6 (TIM4 PWM Channel 1) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternet-function push-pull
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Cau hinh PA6 (TIM3 Input Capture Channel 1) */
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
  TIM_TimeBaseInitStructure.TIM_Period = 65535; // Nên để giá trị lớn nhất
  TIM_TimeBaseInitStructure.TIM_Prescaler = 2 - 1; // Timer Clock
  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
  TIM_Cmd(TIM3, ENABLE);
  TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1); // Filter Input 1
  TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset); // Nếu phát hiện sường lên (rising) thì reset lại giá trị đếm

  /* Cấu hình TIM3 IC Channel 1 (Period) (Direct) */
  TIM_ICInitTypeDef TIM_ICInitStructure;
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; // Timer Clock
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);

  /* Cho phép ngắt TIM3 IC Capture/Compare Channel 1 */
  TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

  /* Cấu hình TIM3 IC Capture Compare (Duty) (Indirect)*/
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; // Timer Clock
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI; // Indirect
  TIM_ICInit(TIM3, &TIM_ICInitStructure);

  /* Cho phép ngắt TIM3 IC Capture/Compare Channel 2 */
  //TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);

  /* ___NVIC___ */

  /* Cấu hình ngắt toàn cục TIM3 */
  NVIC_InitTypeDef NVIC_InitTypeDef_TIM3;
  NVIC_InitTypeDef_TIM3.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitTypeDef_TIM3.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitTypeDef_TIM3.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitTypeDef_TIM3.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitTypeDef_TIM3);

  while (1)
  {
    /* Gửi tần số và độ rộng xung ra usart1 */
    sprintf(strBuffer, "Frequency=%.2fHz, DutyCycle=%.2f%%\r\n", frequency, dutyCycle);
    usart1_putString((uint8_t *)strBuffer);
    delay_ms(1000);
  }
}

/* Trình phục ngắt TIM3  */
void TIM3_IRQHandler(void)
{
  /* Nếu cờ CC1IF = 1 và TIM_IT_CC1 được cho phép */
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
  {
    /* Đọc giá trị Capture Period */
    icValue1 = TIM_GetCapture1(TIM3) + 1;

    if (icValue1 != 0)
    {
      /* Đọc giá trị Capture Duty */
      icValue2 = TIM_GetCapture2(TIM3) + 1;

      /* Tính toán tần sô và độ rộng xung */
      dutyCycle = (icValue2 * 100.0) / icValue1;
      frequency = 72000000 / 2.0 / icValue1;
    }

    /* Xoá cờ CC1IF */
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
  }
}

#include <stm32f10x.h>
#include "..\libraries\delay.h"

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện delay */
  delay_init();

  /* ___GPIO___ */

  /* Bật Clock PortB */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Cấu hình PB6 (TIM4 PWM Channel 1) / PB7 (TIM4 PWM Channel 2) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternet-function push-pull
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

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
  TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
  TIM_Cmd(TIM4, ENABLE);

  /* Cấu hình TIM4 PWM Chanel 1 */
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = 500 - 1;
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  TIM_OC1FastConfig(TIM4, TIM_OCFast_Enable); // Fast PWM
  //TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

  /* Cau hinh TIM4 PWM Chanel 2 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = 500 - 1;
  TIM_OC2Init(TIM4, &TIM_OCInitStructure);
  TIM_OC2FastConfig(TIM4, TIM_OCFast_Enable); // Fast PWM
  //TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

  while (1)
  {
    /* Đặt giá trị Compare (Duty) */
    for (int i = 0; i < 1000; i++)
    {
      TIM_SetCompare1(TIM4, i);
      TIM_SetCompare2(TIM4, 999 - i);
      delay_ms(2);
    }
    delay_ms(1000);

    /* Đặt giá trị Compare (Duty) */
    for (int i = 999; i >= 0; i--)
    {
      TIM_SetCompare1(TIM4, i);
      TIM_SetCompare2(TIM4, 999 - i);
      delay_ms(2);
    }
    delay_ms(1000);
  }
}

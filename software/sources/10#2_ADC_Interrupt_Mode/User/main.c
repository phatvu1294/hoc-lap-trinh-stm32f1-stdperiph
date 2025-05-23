#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

uint16_t adcValue = 0;
char strBuffer[32];

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện delay */
  delay_init();

  /* Khởi tạo thư viện usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Cấu hình PA6 (ADC1 Channel 6) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ___ADC___ */

  /* Bật Clock ADC1 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* Cấu hình Clock ADC */
  RCC_ADCCLKConfig(RCC_PCLK2_Div4);

  /* Cấu hình ADC1 Channel 6 */
  ADC_InitTypeDef ADC_InitStructure;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; // Disable scan mode
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // Enable continous
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // Data align right
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Sofware start trigger
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // Mode Independent
  ADC_InitStructure.ADC_NbrOfChannel = 1; // Number channel
  ADC_Init(ADC1, &ADC_InitStructure);

  /* Cấu hình ADC1 Rank 1 */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);

  /* Cấu hình ngắt ADC1 End of Conversion */
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

  /* Bật ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Hiệu chuẩn điện áp tham chiếu (Vref) */
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1));

  /* ___NVIC___ */

  /* Cho phép ngắt toàn cục ADC1 */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* ___MAIN___ */

  /* Bắt đầu chuyển đổi */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  while (1)
  {
    /* Gửi giá trị ra usart1 */
    sprintf(strBuffer, "ADC Value=%d\r\n", adcValue);
    usart1_putString((uint8_t *)strBuffer);
    delay_ms(100);
  }
}

/* Trình phục vụ ngắt ADC1 */
void ADC1_2_IRQHandler(void)
{
  /* Nếu cờ ADC_SR_EOC được set và ADC_IT_EOC được cho phép */
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
  {
    /* Đọc giá trị ADC1 */
    adcValue = ADC_GetConversionValue(ADC1);

    /* Dừng chuyển đổi */
    //ADC_SoftwareStartConvCmd(ADC1, DISABLE); // Sử dụng cho mode discontinous

    /* Bắt đầu chuyển đổi */
    //ADC_SoftwareStartConvCmd(ADC1, ENABLE); // Sử dụng cho mode discontinous

    /* Xoá cờ ADC_SR_EOC */
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  }
}

#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

uint16_t adcValue;
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

  /* Cau hinh PA6 (ADC1 Channel 6) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ___DMA___ */

  /* Bật Clock DMA1 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Cấu hình DMA Channel 1 (Xem chanel trong bảng 78) */
  DMA_InitTypeDef DMA_InitStructure;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // Đọc từ Periph
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // Disable Memmory to memory (or Periph <-> Memory)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // Enable Circular Mode
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; // Mức độ ưu tiên cao
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adcValue; // Địa chỉ của memory
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // Độ rộng của memory data
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // Enable Memory Increment mode
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x4001244C; // Địa chỉ của periph
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // Độ rộng của periph data
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // Disable Periph Increment mode
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* Cho phép ngắt DMA_Channel 1 */
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

  /* Bật DMA1 Channel 1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* ___TIM___ */

  /* Bật Clock TIM3 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Cấu hình TIM3 */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeDef_TIM3;
  TIM_TimeBaseInitTypeDef_TIM3.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitTypeDef_TIM3.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitTypeDef_TIM3.TIM_Period = 30000 - 1;
  TIM_TimeBaseInitTypeDef_TIM3.TIM_Prescaler = 7200 - 1;
  TIM_TimeBaseInitTypeDef_TIM3.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitTypeDef_TIM3);

  /* Bật TIM3 */
  TIM_Cmd(TIM3, ENABLE);

  /* Cấu hình TIM3 Trigger Output */
  TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Disable); // Disable slave mode
  TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update); // Source TRGO event update

  /* ____ADC___ */

  /* Bật Clock ADC1 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* Cấu hình Clock ADC */
  RCC_ADCCLKConfig(RCC_PCLK2_Div4);

  /* Cấu hình ADC1 Channel 6 */
  ADC_InitTypeDef ADC_InitTypeDef_CHANEL6;
  ADC_InitTypeDef_CHANEL6.ADC_ScanConvMode = DISABLE; // Disable scan mode
  ADC_InitTypeDef_CHANEL6.ADC_ContinuousConvMode = DISABLE; // Disable continous
  ADC_InitTypeDef_CHANEL6.ADC_DataAlign = ADC_DataAlign_Right; // Data align right
  ADC_InitTypeDef_CHANEL6.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; // Tim 3 trigger
  ADC_InitTypeDef_CHANEL6.ADC_Mode = ADC_Mode_Independent; // Mode Independent
  ADC_InitTypeDef_CHANEL6.ADC_NbrOfChannel = 1; // Number channel
  ADC_Init(ADC1, &ADC_InitTypeDef_CHANEL6);

  /* Cấu hình ADC1 Rank 1 */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);

  /* Cho phép ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Bật ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Hiệu chuẩn điện áp tham chiếu (Vref) */
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1));

  /* Cho phép ngắt toàn cục DMA1 Channel 1 */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* ___MAIN___ */

  /* Cho phép chuyển đổi ADC1 Trigger từ TIM3 Update Event */
  ADC_ExternalTrigConvCmd(ADC1, ENABLE);

  while (1)
  {

  }
}

/* Trình phục vụ ngắt DMA1 Channel 1 */
void DMA1_Channel1_IRQHandler(void)
{
  /* Nếu cờ DMA_SR_TC được set và DMA1_IT_TC1 được cho phép */
  if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
  {
    /* Gửi giá trị ra usart1 */
    sprintf(strBuffer, "ADC Value=%d\r\n", adcValue);
    usart1_putString((uint8_t *)strBuffer);

    /* Xoá cờ DMA_SR_TC */
    DMA_ClearITPendingBit(DMA1_IT_TC1);
  }
}

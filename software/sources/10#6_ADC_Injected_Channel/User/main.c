#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

/* ADC1 Channel 6 -> Đọc ADC chế độ DMA Interrupt */
/* ADC1 Channel 7 -> Đọc ADC chế độ Interrupt, kích hoạt mỗi khi TIM4 TRGO Update */
/* ADC1 Channel 7 (Injected) có mức ưu tiên ngắt cao hơn, nên sẽ làm gián đoạn ADC Channel 6 (Regular) */
/* Khi ngắt ADC1 Channel 7 kết thúc, ADC1 Channel 6 sẽ tiếp tục */
/* Chế độ này giống với Interrupt nhưng chỉ dành cho ADC */

uint16_t regularADCValue;
uint16_t injectedADCValue;
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

  /* Cấu hình PA7 (ADC1 Channel 7) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
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
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&regularADCValue; // Địa chỉ của memory
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

  /* Bật Clock TIM4 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* Cấu hình TIM4 */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeDef_TIM4;
  TIM_TimeBaseInitTypeDef_TIM4.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitTypeDef_TIM4.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitTypeDef_TIM4.TIM_Period = 30000 - 1;
  TIM_TimeBaseInitTypeDef_TIM4.TIM_Prescaler = 7200 - 1;
  TIM_TimeBaseInitTypeDef_TIM4.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitTypeDef_TIM4);

  /* Bật TIM4 */
  TIM_Cmd(TIM4, ENABLE);

  /* Cấu hình TIM4 Trigger Output */
  TIM_SelectMasterSlaveMode(TIM4, TIM_MasterSlaveMode_Disable); // Disable slave mode
  TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update); // Source TRGO event update

  /* ____ADC___ */

  /* Bật Clock ADC1 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* Cấu hình Clock ADC */
  RCC_ADCCLKConfig(RCC_PCLK2_Div4);

  /* Cấu hình ADC1 Channel 6 */
  ADC_InitTypeDef ADC_InitStructure;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; // Disable scan mode
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // Enable continous
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // Data align right
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // sofware trigger
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // Mode Independent
  ADC_InitStructure.ADC_NbrOfChannel = 1; // Number channel
  ADC_Init(ADC1, &ADC_InitStructure);

  /* Cấu hình ADC1 Rank 1 Regular */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);

  /* Cho phép ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Cấu hình ADC1 Rank 1 Injected */
  ADC_InjectedSequencerLengthConfig(ADC1, 1); // Số lượng kênh Injected
  ADC_InjectedChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5);
  ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T4_TRGO); // Kích ngưỡng Injected TIM4

  /* Cho phép ngắt ADC1 Injected EOC */
  ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);

  /* Bật ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Hiệu chuẩn điện áp tham chiếu (Vref) */
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1));

  /* ___NVIC___ */

  /* Cho phép ngắt toàn cục DMA1 Channel 1 */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* Cho phép ngắt toàn cục ADC1 */
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* ___MAIN___ */

  /* Bắt đầu chuyển đổi ADC1 Software trigger (ADC1 Channel 6) */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  /* Bắt đầu chuyển đổi ADC1 Trigger 4 update event (ADC1 Channel 7) */
  ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

  while (1)
  {

  }
}

/* Trình phục vụ ngắt DMA1 Chanel 1 */
void DMA1_Channel1_IRQHandler(void)
{
  /* Nếu cờ DMA_SR_TC được set và DMA_IT_TC được cho phép */
  if (DMA_GetITStatus(DMA_IT_TC) != RESET)
  {
    /* Gửi giá trị ra usart1 */
    sprintf(strBuffer, "ADC CH6 Regular Value=%d\r\n", regularADCValue);
    usart1_putString((uint8_t *)strBuffer);

    /* Xóa cờ DMA_IT_TC */
    DMA_ClearITPendingBit(DMA_IT_TC);
  }
}

/* Trình phục vụ ngắt ADC1 */
void ADC1_2_IRQHandler(void)
{
  /* Nếu cờ ADC_SR_JEOC được set và ADC1_IT_JEOC được cho phép */
  if (ADC_GetITStatus(ADC1, ADC_IT_JEOC) != RESET)
  {
    /* Đọc giá trị Injeted ADC1 Injected Channel 1 */
    injectedADCValue = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);

    /* Gửi giá trị ra usart1 */
    sprintf(strBuffer, "ADC CH7 Injected Value=%d\r\n", injectedADCValue);
    usart1_putString((uint8_t *)strBuffer);

    /* Xóa cờ ADC_SR_JEOC */
    ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
  }
}

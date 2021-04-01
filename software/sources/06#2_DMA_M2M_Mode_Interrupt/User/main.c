#include <stm32f10x.h>

#define BUFFER_SIZE 4

uint8_t tranferComplete = 0;

/* Lưu trữ dữ liệu vào bộ nhớ Flash */
uint32_t srcBuffer[BUFFER_SIZE] = {0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD};
uint32_t dstBuffer[BUFFER_SIZE];

uint8_t buffer_compare(const uint32_t* pBuffer, uint32_t* pBuffer1, uint16_t bufferLength);
void DMA1_Channel1_IRQHandler(void);

int main(void)
{
  /* ___GPIO___ */

  /* Bật Clock PortC */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

  /* Set PC13 */
  GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_SET);

  /* Cấu hình PC13 */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Bật Clock DMA */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Deinit DMA1 */
  DMA_DeInit(DMA1_Channel1);

  /* Cấu hình DMA1 */
  DMA_InitTypeDef DMA_InitStructure;
  DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE; // Kích thước bộ nhớ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // Ngoại vi là nguồn
  DMA_InitStructure.DMA_M2M = DMA_M2M_Enable; // Memory to memory Mode
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&srcBuffer[0]; // Địa chỉ bộ nhớ nguồn
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 4 byte
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable; // Tự động tăng bộ nhớ ngoại vi
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&dstBuffer[0]; // Địa chỉ bộ nhớ đích
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; // 4 byte
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // Tự động tăng bộ nhớ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; // Chế độ bình thường
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; // Mức độ ưu tiên cao
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* Cho phép ngắt DMA1 TC (Transfer Complete) */
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

  /* Bật DMA1 Channel 1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* ___NVIC___ */

  /* Cấu hình ngắt toàn cục DMA1 Channel 1 */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* ___MAIN___ */

  /* Chờ cho đến khi truyền thành công */
  while (tranferComplete == 0);

  /* Nếu Buffer nguồn (Flash) và Buffer đích (SRAM) giống nhau */
  if (buffer_compare((uint32_t *)srcBuffer, (uint32_t *)dstBuffer, BUFFER_SIZE) == 1)
  {
    /* Reset PC13 */
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
  }
  else
  {
    /* Set PC13 */
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
  }

  while (1)
  {

  }
}

/* Hàm so sánh hai Buffer */
uint8_t buffer_compare(const uint32_t* pBuffer, uint32_t* pBuffer1, uint16_t bufferLength)
{
  while (bufferLength--)
  {
    if (*pBuffer != *pBuffer1)
    {
      return 0;
    }
    pBuffer++;
    pBuffer1++;
  }

  return 1;
}

/* Trình phục vụ ngắt DMA1 Channel 1 */
void DMA1_Channel1_IRQHandler(void)
{
  /* Kiểm tra cờ TC1 */
  if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
  {
    /* Truyền thành công */
    tranferComplete = 1;

    /* Xoá cờ DMA1_Channel1 Half Transfer, Transfer Complete và Global */
    DMA_ClearITPendingBit(DMA1_IT_GL1);
  }
}

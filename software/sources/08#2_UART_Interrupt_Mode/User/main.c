#include <stm32f10x.h>
#include <string.h>

uint8_t rxBuffer[128];
uint8_t rxIndex = 0;
uint8_t rxComplete = 0;

void usart1_putChar(uint8_t c);
void usart1_putString(uint8_t *s);
void USART1_IRQHandler(void);

int main(void)
{
  /* ___GPIO___ */

  /* Bật Clock PortA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Cấu hình PA10 (USART1 RX) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Cấu hình PA9 (USART1 TX) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ___AFIO___ */

  /* Bật clock AFIO để remap lại chân USART1, Xem tài liệu TX:PA9/PB6(remap), RX:PA10/PB7(remap) */
  /* Phải cấu hình chân GPIO về PB6 và PB7 */
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  //GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  /* ___USART___ */

  /* Bật Clock USART1 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /* Cấu hình USART1 */
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);

  /* Cho phép ngắt USART1 RXNE */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  /* ___NVIC___ */

  /* Cấu hình ngắt toàn cục USART1 */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  while (1)
  {
    /* Nếu nhận dữ liệu thành công */
    if (rxComplete == 1)
    {
      /* Gửi chuỗi (line) nhận được ra USART1 */
      usart1_putString(rxBuffer);

      /* Xoá Buffer, reset biến đếm, cho phép ngắt USART RXNE */
      memset(rxBuffer, 0, sizeof(rxBuffer));
      rxComplete = 0;
      rxIndex = 0;
      USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    }
  }
}

/* Hàm gửi ký tự ra USART1 */
void usart1_putChar(uint8_t c)
{
  /* Gửi từng byte ra USART1 */
  USART_SendData(USART1, c);

  /* Chờ cho đến khi USART1 data rỗng */
  /* Cờ TXE = 1: USART1 truyền thành công và USART1 data lúc này rỗng */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/* Hàm gửi chuỗi ra USART1 */
void usart1_putString(uint8_t *s)
{
  uint8_t i = 0;

  /* Nếu không phải là ký tự null */
  while (s[i] != 0)
  {
    /* Gửi ký tự ra USART1 */
    usart1_putChar(s[i++]);
  }
}

/* Trình phục vụ ngắt USART1 */
void USART1_IRQHandler(void)
{
  /* Nếu trạng thái cờ RXNE = 1 và ngắt USART_IT_RXNE được cho phép */
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    /* Lưu dữ liệu USART1 vào RxChar */
    uint8_t rxChar = USART_ReceiveData(USART1);

    /* Thêm dồn RxChar vào RxBuffer */
    rxBuffer[rxIndex++] = rxChar;

    /* Nếu RxChar là ký tự xuống dòng */
    if (rxChar == '\n')
    {
      /* Nhận thành công */
      rxComplete = 1;

      /* Cấm ngắt USART1 RXNE */
      USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    }

    /* Xoá cờ RXNE */
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}

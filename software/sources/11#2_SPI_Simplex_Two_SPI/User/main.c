#include <stm32f10x.h>
#include <string.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

#define BUFFER_SIZE 10

uint8_t txBuffer[BUFFER_SIZE] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
uint8_t rxBuffer[BUFFER_SIZE];
uint8_t rxIndex = 0;
uint8_t rxComplete = 0;

uint32_t prevMillis = 0;

void spi1_transmit(uint8_t *pData, uint8_t dataSize);
void SPI2_IRQHandler(void);

/* SPI1 TX -> SPI2 RX: Simplex Communication */
/* MOSI   <->    MISO */
/* SCK    <->    SCK  */
/* PB0    <->    NSS  */

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện delay */
  delay_init();

  /* Khởi tạo thư viện usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortA và PortB */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

  /* Cấu hình PA7 (SPI1 MOSI) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Cấu hình PA5 (SPI1 CLK) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Cấu hình PB0 (SPI1 NSS) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);

  /* Cấu hình PB14 (SPI2 MISO) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Cấu hình PB13 (SPI2 CLK) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Cấu hình PB12 (SPI2 NSS) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* ___SPI___ */

  /* ___SPI1 Mode TX___ */

  /* Bật Clock SPI1 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  /* Cấu hình SPI1 */
  SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; // Prescaler
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; // Clock Phase
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; // Clock Polarity LOW
  SPI_InitStructure.SPI_CRCPolynomial = 0; // 0
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // Data size 8bit
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx; // Simplex
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // MSB First
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; // Mode Master
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // NSS Signal Software
  SPI_Init(SPI1, &SPI_InitStructure);
  SPI_CalculateCRC(SPI1, DISABLE); // Disable CRC Calculation

  /* Bật SPI1 */
  SPI_Cmd(SPI1, ENABLE);

  /* ___SPI2 Mode RX Interrupt___ */

  /* Bật Clock SPI2 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  /* Cấu hình SPI2 */
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; // Prescaler
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; // Clock Phase
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; // Clock Polarity LOW
  SPI_InitStructure.SPI_CRCPolynomial = 0; // 0
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // Data size 8bit
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Rx; // Simplex
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // MSB First
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave; // Mode Master
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard; // NSS Signal Hardware
  SPI_Init(SPI2, &SPI_InitStructure);
  SPI_CalculateCRC(SPI2, DISABLE); // Disable CRC Calculation

  /* Cho phép ngắt SPI2 RXNE */
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);

  /* Bật SPI2 */
  SPI_Cmd(SPI2, ENABLE);

  /* ___NVIC___ */

  /* Cấu hình ngắt toàn cục SPI2 */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  while (1)
  {
    /* Đọc thời điểm hiện tại */
    uint32_t currMillis = millis();

    /* delay non-blocking 1000ms để SPI1 truyền dữ liệu */
    if (currMillis - prevMillis >= 1000)
    {
      prevMillis = currMillis;

      /* Kéo chân NSS xuống mức thấp */
      GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);

      /* Chờ 1 khoảng thời gian */
      delay_ms(1);

      /* Truyền dữ liệu ra SPI1 */
      spi1_transmit(txBuffer, BUFFER_SIZE);

      /* Chờ 1 khoảng thời gian */
      delay_ms(1);

      /* Kéo chân NSS lên mức cao */
      GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
    }

    /* Nếu nhận dữ liệu thành công */
    if (rxComplete == 1)
    {
      /* Gửi từng ký tự Buffer ra usart1 */
      for (int i = 0; i < sizeof(rxBuffer); i++)
      {
        usart1_putChar(rxBuffer[i]);
      }

      /* Xóa Buffer, reset biến đếm và cho phép ngắt SPI1 RXNE */
      memset(rxBuffer, 0, BUFFER_SIZE);
      rxComplete = 0;
      rxIndex = 0;
      SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
    }
  }
}

/* Trình phục vụ ngắt SPI2 */
void SPI2_IRQHandler(void)
{
  /* Nếu cờ RXNE = 1 và ngắt SPI2 RXNE được cho phép */
  if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE) != RESET)
  {
    /* Đọc dữ liệu nhận được từ SPI2 */
    rxBuffer[rxIndex] = SPI_I2S_ReceiveData(SPI2);

    /* Tăng biến đệm RX */
    rxIndex++;

    /* Nếu biến đếm bằng kích thước bộ đệm */
    if (rxIndex == BUFFER_SIZE)
    {
      /* Nhận dữ liệu thành công */
      rxComplete = 1;

      /* Cấm ngắt SPI2 RXNE */
      SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
    }

    /* Xóa cờ RXNE */
    SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_IT_RXNE);
  }
}

/* Hàm gửi dữ liệu ra SPI1 */
void spi1_transmit(uint8_t *pData, uint8_t dataSize)
{
  /* Biến đếm Tx */
  uint8_t txIndex = 0;

  /* Nếu kích thước dữ liệu gửi khác 0 */
  while (dataSize != 0)
  {
    /* Gửi từng byte vào SPI1 */
    SPI_I2S_SendData(SPI1, pData[txIndex]);

    /* Chờ cho đến khi SPI1 Data rỗng */
    /* Cờ TXE = 1: SPI1 truyền thành công và SPI1 data lúc này rỗng */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* Tăng giảm biến đếm */
    txIndex++;
    dataSize--;
  }
}

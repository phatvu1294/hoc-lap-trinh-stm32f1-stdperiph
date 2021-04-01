#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\usart1.h"

uint8_t data;
char txBuffer[32];

void TIM1_UP_IRQHandler(void);
void USB_HP_CAN1_TX_IRQHandler(void);
void USB_LP_CAN1_RX0_IRQHandler(void);

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Cấu hình PA12 (CAN_TX) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Cấu hình PA11 (CAN_RX) */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ___TIM___ */

  /* Bật Clock Timer */
  RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1EN, ENABLE);

  /* Cấu hinh TIM1 */
  /* f_clock_source/f_update = f_update * Prescaler * (Counter_Period + 1) */
  /* có f_clock_source = 72MHz */
  /* Với f_update = 0.3333Hz => 72000000 = Prescaler * (Counter_Period + 1) */
  /* ta có Counter_Period (max) = 2^16 - 1 = 65535 => Prescaler (min) = 3296 */
  /* Chọn Prescaler = 7200 => Counter_Period = 30000-1 */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeDef_TIM1;
  TIM_TimeBaseInitTypeDef_TIM1.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitTypeDef_TIM1.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitTypeDef_TIM1.TIM_Period = 30000 - 1; // 3s
  TIM_TimeBaseInitTypeDef_TIM1.TIM_Prescaler = 7200 - 1;
  TIM_TimeBaseInitTypeDef_TIM1.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitTypeDef_TIM1);
  TIM_Cmd(TIM1, ENABLE);

  /* Cho phép ngắt TIM1 Update */
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

  /* ___CAN___ */

  /* Bật Clock CAN1 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

  /* Truy cập vào địa chỉ sau để tính toán: */
  /* http://www.bittiming.can-wiki.info/ */

  /* Cấu hình CAN1 */
  CAN_InitTypeDef CAN_InitStructure;
  CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack; // Mode LoopBack
  CAN_InitStructure.CAN_Prescaler = 2; // CAN Prescaler
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq; // Bit timre quantum
  CAN_InitStructure.CAN_BS1 = CAN_BS1_15tq; //Bit segment 1
  CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq; //Bit segment 2
  CAN_InitStructure.CAN_NART = ENABLE; // Enable no-automatic retransmission mode
  CAN_InitStructure.CAN_ABOM = ENABLE; // Enable automatic bus-off management
  CAN_InitStructure.CAN_AWUM = DISABLE; // Disable the automatic wake-up mode
  CAN_InitStructure.CAN_RFLM = DISABLE; // Disable Receive FIFO Locked mode
  CAN_InitStructure.CAN_TTCM = DISABLE; // Disable time triggered communication mode
  CAN_InitStructure.CAN_TXFP = DISABLE; // Disable transmit FIFO priority
  CAN_Init(CAN1, &CAN_InitStructure);

  /* Cấu hình ngắt CAN TX Mailbox 0, 1, 2 Transmit complete */
  CAN_ITConfig(CAN1, CAN_IT_RQCP0, ENABLE);

  /* Cấu hình ngắt CAN FIFO0 Messgae Pending */
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

  /* ___CAN RX Filter___ */

  /* Cấu hình lọc chấp nhận CAN */
  CAN_FilterInitTypeDef CAN_FilterInitStructure;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInitStructure.CAN_FilterNumber = 0; // Bank
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInit(&CAN_FilterInitStructure);

  /* ___NVIC___ */

  /* Cấu hình ngắt CAN toàn cục */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  /* Cấu hình ngắt TIM1 toàn cục */
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  while (1)
  {

  }
}

/* Trình phục vụ ngắt TIM1 Update */
void TIM1_UP_IRQHandler(void)
{
  /* Nếu là ngắt TIM1 Update */
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
  {
    /* ___TX Function___ */

    /* CAN TX Message */
    CanTxMsg CanTxMsgStructure;
    CanTxMsgStructure.ExtId = 0;
    CanTxMsgStructure.StdId = 0x65D;
    CanTxMsgStructure.IDE = CAN_Id_Standard;
    CanTxMsgStructure.RTR = CAN_RTR_Data;
    CanTxMsgStructure.DLC = 1;
    CanTxMsgStructure.Data[0] = data + 0x30;

    /* Đẩy dữ liệu lên bus */
    uint8_t transmitMailbox = CAN_Transmit(CAN1, &CanTxMsgStructure);

    /* Chờ cho đến khi gửi thành công */
    while (CAN_TransmitStatus(CAN1, transmitMailbox) != CAN_TxStatus_Ok);

    /* Nếu quá giới hạn thì đặt lại */
    if (++data > 2) data = 0;

    /* Xoá cờ software */
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  }
}

/* Trình phục vụ ngắt CAN Transmit (TX) */
void USB_HP_CAN1_TX_IRQHandler(void)
{
  /* Nếu là ngắt TX Mailbox0 Transmit complete */
  if (CAN_GetITStatus(CAN1, CAN_IT_RQCP0) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Message trasmit complete: Mailbox0\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_RQCP0);
  }
}

/* Trình phục vụ ngắt CAN FIFO0 (RX0) */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* Nếu là ngắt RX FIFO0 Message Pending */
  if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Message receive complete: FIFO0\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* ___RX Function___ */

    /* CAN RX Message */
    CanRxMsg CanRXMsgStructure;
    CanRXMsgStructure.ExtId = 0;
    CanRXMsgStructure.StdId = 0;
    CanRXMsgStructure.IDE = CAN_Id_Standard;
    CanRXMsgStructure.RTR = CAN_RTR_Data;
    CanRXMsgStructure.DLC = 1;
    CanRXMsgStructure.FMI = 0;
    CanRXMsgStructure.Data[0] = 0;

    /* Nhận dữ liệu */
    CAN_Receive(CAN1, CAN_FIFO0, &CanRXMsgStructure);

    /* Gửi dữ liệu nhận được ra usart 1 */
    usart1_putString((uint8_t *)"\r\nCAN Standard ID: ");
    sprintf(txBuffer, "%d\r\n", CanRXMsgStructure.StdId);
    usart1_putString((uint8_t *)txBuffer);
    usart1_putString((uint8_t *)"CAN Data Length: ");
    sprintf(txBuffer, "%d\r\n", CanRXMsgStructure.DLC);
    usart1_putString((uint8_t *)txBuffer);
    usart1_putString((uint8_t *)"CAN Data: ");
    sprintf(txBuffer, "%s\r\n\r\n", CanRXMsgStructure.Data);
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
  }
}

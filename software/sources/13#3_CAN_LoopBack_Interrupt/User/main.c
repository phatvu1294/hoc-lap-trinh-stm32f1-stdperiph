#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\usart1.h"

char txBuffer[32];

void USB_HP_CAN1_TX_IRQHandler(void);
void USB_LP_CAN1_RX0_IRQHandler(void);
void CAN1_RX1_IRQHandler(void);
void CAN1_SCE_IRQHandler(void);

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
  CAN_InitStructure.CAN_ABOM = DISABLE; // Enable automatic bus-off management
  CAN_InitStructure.CAN_AWUM = DISABLE; // Disable the automatic wake-up mode
  CAN_InitStructure.CAN_RFLM = DISABLE; // Disable Receive FIFO Locked mode
  CAN_InitStructure.CAN_TTCM = DISABLE; // Disable time triggered communication mode
  CAN_InitStructure.CAN_TXFP = DISABLE; // Disable transmit FIFO priority
  CAN_Init(CAN1, &CAN_InitStructure);

  /* Cấu hình ngắt CAN TX Mailbox 0, 1, 2 Transmit complete */
  CAN_ITConfig(CAN1, CAN_IT_RQCP0, ENABLE);
  CAN_ITConfig(CAN1, CAN_IT_RQCP1, ENABLE);
  CAN_ITConfig(CAN1, CAN_IT_RQCP2, ENABLE);

  /* Cấu hình ngắt CAN FIFO0 Messgae Pending */
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

  /* Cấu hình ngắt CAN Error */
  CAN_ITConfig(CAN1, CAN_IT_EPV, ENABLE);
  CAN_ITConfig(CAN1, CAN_IT_EWG, ENABLE);
  CAN_ITConfig(CAN1, CAN_IT_BOF, ENABLE);

  /* ___CAN RX Filter___ */

  /* Cấu hình lọc chấp nhận CAN RX */
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

  /* ___TX Function___ */

  /* CAN TX Message */
  CanTxMsg CanTxMsgStructure;
  CanTxMsgStructure.ExtId = 0;
  CanTxMsgStructure.StdId = 0x65D;
  CanTxMsgStructure.IDE = CAN_Id_Standard;
  CanTxMsgStructure.RTR = CAN_RTR_Data;
  CanTxMsgStructure.DLC = 5;
  CanTxMsgStructure.Data[0] = 'H';
  CanTxMsgStructure.Data[1] = 'E';
  CanTxMsgStructure.Data[2] = 'L';
  CanTxMsgStructure.Data[3] = 'L';
  CanTxMsgStructure.Data[4] = 'O';

  /* Đẩy dữ liệu lên bus */
  uint8_t transmitMailbox = CAN_Transmit(CAN1, &CanTxMsgStructure);

  /* Chờ cho đến khi gửi thành công */
  while (CAN_TransmitStatus(CAN1, transmitMailbox) != CAN_TxStatus_Ok);

  /* ___RX Function___ */

  /* CAN RX Message */
  CanRxMsg CanRXMsgStructure;
  CanRXMsgStructure.ExtId = 0;
  CanRXMsgStructure.StdId = 0;
  CanRXMsgStructure.IDE = CAN_Id_Standard;
  CanRXMsgStructure.RTR = CAN_RTR_Data;
  CanRXMsgStructure.DLC = 5;
  CanRXMsgStructure.FMI = 0;
  CanRXMsgStructure.Data[0] = 0;
  CanRXMsgStructure.Data[1] = 0;
  CanRXMsgStructure.Data[2] = 0;
  CanRXMsgStructure.Data[3] = 0;
  CanRXMsgStructure.Data[4] = 0;

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

  /* ___NVIC___ */

  /* Cấu hình ngắt toàn cục CAN */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  while (1)
  {

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

  /* Nếu là ngắt TX Mailbox1 Transmit complete */
  if (CAN_GetITStatus(CAN1, CAN_IT_RQCP1) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Message trasmit complete: Mailbox1\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_RQCP1);
  }

  /* Nếu là ngắt TX Mailbox2 Transmit complete */
  if (CAN_GetITStatus(CAN1, CAN_IT_RQCP2) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Message trasmit complete: Mailbox2\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_RQCP2);
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

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
  }
}

/* Trình phục vụ ngắt CAN FIFO1 (RX1) */
void CAN1_RX1_IRQHandler(void)
{
  /* Làm gì đó ở đây */
}

/* Trình phục vụ ngắt CAN Status Change Error (SCE) */
void CAN1_SCE_IRQHandler(void)
{
  /* Nếu là ngắt lỗi EPV */
  if (CAN_GetITStatus(CAN1, CAN_IT_EPV) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Error: EPV\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);
  }

  /* Nếu là ngắt lỗi EWG */
  if (CAN_GetITStatus(CAN1, CAN_IT_EWG) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Error: EWG\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_EWG);
  }

  /* Nếu là ngắt lỗi BOF */
  if (CAN_GetITStatus(CAN1, CAN_IT_BOF) != RESET)
  {
    /* Gửi tin nhắn ra usart1 */
    sprintf(txBuffer, "Error: BOF Bus-off\r\n");
    usart1_putString((uint8_t *)txBuffer);

    /* Xoá cờ bằng software */
    CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);
  }
}

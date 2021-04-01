#include <stm32f10x.h>
#include <stdio.h>
#include "..\libraries\usart1.h"

char txBuffer[32];

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện usart1 */
  usart1_init();

  /* ___GPIO___ */

  /* Bật Clock PortA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Cấu hình PA12 (CAN_TX) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
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
  usart1_putString((uint8_t *)"CAN Standard ID: ");
  sprintf(txBuffer, "%d\r\n", CanRXMsgStructure.StdId);
  usart1_putString((uint8_t *)txBuffer);
  usart1_putString((uint8_t *)"CAN Data Length: ");
  sprintf(txBuffer, "%d\r\n", CanRXMsgStructure.DLC);
  usart1_putString((uint8_t *)txBuffer);
  usart1_putString((uint8_t *)"CAN Data: ");
  sprintf(txBuffer, "%s\r\n\r\n", CanRXMsgStructure.Data);
  usart1_putString((uint8_t *)txBuffer);

  while (1)
  {

  }
}

#include <stm32f10x.h>

int main(void)
{
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

  while (1)
  {

  }
}

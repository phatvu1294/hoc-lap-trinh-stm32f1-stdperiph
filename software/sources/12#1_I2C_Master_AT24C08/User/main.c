#include <stm32f10x.h>

#define AT24C08_ADDR 0x50 | (1 << 0)

/* Hàm ghi byte vào AT24C08 */
void eeprom_writeByte(uint8_t address, uint16_t wordAddress, uint8_t data);
void eeprom_writePage(uint8_t address, uint16_t wordAddress, uint8_t *data, uint16_t length);
void eeprom_readByte(uint8_t address, uint16_t wordAddress, uint8_t *data);

/* Nguyên mẫu hàm I2C */
void i2c_start(void);
void i2c_stop(void);
void i2c_addressDirection(uint8_t address, uint8_t direction);
void i2c_transmit(uint8_t byte);
uint8_t i2c_receiveAck(void);
uint8_t i2c_receiveNack(void);
void i2c_write(uint8_t address, uint8_t data);
void i2c_read(uint8_t address, uint8_t *data);

int main(void)
{
  /* ___I2C___ */

  /* Bật Clock I2C2 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

  /* Cấu hình I2C2 */
  I2C_InitTypeDef I2C_InitStructure;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = 100000; // 100kHz
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_Init(I2C2, &I2C_InitStructure);

  /* Bật I2C2 */
  I2C_Cmd(I2C2, ENABLE);

  /* ___GPIO___ */

  /* Bật Clock PortB */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Cấu hình PB11 (SDA), PB10 (SCL) */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ___MAIN___ */
	
	/* Ghi dữ liệu vào EEPROM */
	eeprom_writeByte(AT24C08_ADDR, 0x0000, 0xAA);

  while (1)
  {
		/* Đọc dữ liệu từ EEPROM */
		uint8_t data;
		eeprom_readByte(AT24C08_ADDR, 0x0000, &data);
  }
}

/* Hàm ghi byte vào AT24C08 */
void eeprom_writeByte(uint8_t address, uint16_t wordAddress, uint8_t data)
{
  i2c_start();
  i2c_addressDirection(address << 1, I2C_Direction_Transmitter);
  i2c_transmit((wordAddress >> 8) & 0xFF);
	i2c_transmit(wordAddress & 0xFF);
  i2c_transmit(data);
  i2c_stop();
}

/* Hàm ghi page vào AT24C08 */
void eeprom_writePage(uint8_t address, uint16_t wordAddress, uint8_t *data, uint16_t length)
{
  i2c_start();
  i2c_addressDirection(address << 1, I2C_Direction_Transmitter);
  i2c_transmit((wordAddress >> 8) & 0xFF);
	i2c_transmit(wordAddress & 0xFF);
  for (uint16_t i = 0; i < length; i++)
  {
    i2c_transmit(data[i]);
  }
  i2c_stop();
}

/* Hàm đọc byte từ AT24C08 */
void eeprom_readByte(uint8_t address, uint16_t wordAddress, uint8_t *data)
{
	i2c_start();
  i2c_addressDirection(address << 1, I2C_Direction_Transmitter);
  i2c_transmit((wordAddress >> 8) & 0xFF);
	i2c_transmit(wordAddress & 0xFF);
	i2c_start();
  i2c_addressDirection(address << 1, I2C_Direction_Receiver);
	*data = i2c_receiveNack();
	i2c_stop();
}

/* Hàm tạo điều kiện START */
void i2c_start(void)
{
  /* Chờ cho đến khi I2C không bận */
  while (I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY));

  /* Tạo điều khiện START */
  I2C_GenerateSTART(I2C2, ENABLE);

  /* Chờ I2C EV5, nghĩa là điều kiện START chính xác đã được tạo trên bus I2C */
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
}

/* Hàm tạo điều kiện STOP */
void i2c_stop(void)
{
  /* Tạo điều khiện STOP */
  I2C_GenerateSTOP(I2C2, ENABLE);

  /* Chờ cho đến khi điều kiện STOP được kết thúc */
  while (I2C_GetFlagStatus(I2C2, I2C_FLAG_STOPF));

  /* Xóa cờ I2C STOP */
  I2C_ClearFlag(I2C2, I2C_FLAG_STOPF);
}

/* Hàm gửi địa chỉ và điều hướng bus */
void i2c_addressDirection(uint8_t address, uint8_t direction)
{
  /* Gửi địa chỉ I2C */
  I2C_Send7bitAddress(I2C2, address, direction);

  /* Chờ I2C EV6, nghĩa là slave đã chấp nhận địa chỉ */
  if (direction == I2C_Direction_Transmitter)
  {
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  }
  else if (direction == I2C_Direction_Receiver)
  {
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  }
}

/* Hàm truyền dữ liệu */
void i2c_transmit(uint8_t byte)
{
  /* Gửi dữ liệu I2C */
  I2C_SendData(I2C2, byte);

  /* Chờ I2C EV8_2, nghĩa là dữ liệu đã được chuyển ra ngoài trên đường bus */
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* Hàm nhận dữ liệu ACK */
uint8_t i2c_receiveAck(void)
{
  /* Cho phép ACK để nhận dữ liệu */
  I2C_AcknowledgeConfig(I2C2, ENABLE);

  /* Chờ I2C EV7, nghĩa là dữ liệu đã nhận được trong thanh ghi i2c */
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));

  /* Trả về dữ liệu nhận được từ thanh ghi */
  return I2C_ReceiveData(I2C2);
}

/* Hàm nhận dữ liệu NACK */
uint8_t i2c_receiveNack(void)
{
  /* Tắt ACK để nhận dữ liệu */
  I2C_AcknowledgeConfig(I2C2, DISABLE);

  /* Chờ I2C EV7, nghĩa là dữ liệu đã nhận được trong thanh ghi i2c */
  while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));

  /* Trả về dữ liệu nhận được từ thanh ghi */
  return I2C_ReceiveData(I2C2);
}

/* Hàm ghi dữ liệu */
void i2c_write(uint8_t address, uint8_t data)
{
  i2c_start();
  i2c_addressDirection(address << 1, I2C_Direction_Transmitter);
  i2c_transmit(data);
  i2c_stop();
}

/* Hàm đọc dữ liệu */
void i2c_read(uint8_t address, uint8_t *data)
{
  i2c_start();
  i2c_addressDirection(address << 1, I2C_Direction_Receiver);
  *data = i2c_receiveNack();
  i2c_stop();
}

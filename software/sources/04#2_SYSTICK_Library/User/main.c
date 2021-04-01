#include <stm32f10x.h>
#include "..\libraries\delay.h"

uint32_t prevMillis1 = 0;
uint32_t prevMillis2 = 0;

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện delay */
  delay_init();

  /* ___GPIO___ */

  /* Bật Clock PortC */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

  /* Cấu hình PC13 và PC14 */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Set PC13 và PC14 */
  GPIO_WriteBit(GPIOC, GPIO_Pin_13 | GPIO_Pin_14, Bit_SET);

  while (1)
  {
    /* Chớp tắt LED delay blocking */
    //GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    //delay_ms(100);
    //GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
    //delay_ms(100);

    /* Chớp tắt LED delay non-blocking */
    static uint8_t ledState1 = 1;
    static uint8_t ledState2 = 1;
    uint32_t currMillis = millis();

    if (currMillis - prevMillis1 >= 500)
    {
      prevMillis1 = currMillis;
      ledState1 = !ledState1;
      GPIO_WriteBit(GPIOC, GPIO_Pin_13, ledState1 ? Bit_SET : Bit_RESET);
    }

    if (currMillis - prevMillis2 >= 1000)
    {
      prevMillis2 = currMillis;
      ledState2 = !ledState2;
      GPIO_WriteBit(GPIOC, GPIO_Pin_14, ledState2 ? Bit_SET : Bit_RESET);
    }
  }
}

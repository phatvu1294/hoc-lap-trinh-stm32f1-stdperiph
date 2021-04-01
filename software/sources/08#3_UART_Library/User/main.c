#include <stm32f10x.h>
#include "..\libraries\delay.h"
#include "..\libraries\usart1.h"

int main(void)
{
  /* ___LIB___ */

  /* Khởi tạo thư viện delay */
  delay_init();

  /* Khởi tạo thư viện usart1 */
  usart1_init();

  while (1)
  {
    /* Gửi chuỗi ra usart1 */
    usart1_putString((uint8_t *)"Hello World!\r\n");
    delay_ms(1000);
  }
}

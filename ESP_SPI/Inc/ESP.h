#include "main.h"

#define ESP_END  0xFF
#define ESP_IDLE 0
#define ESP_INIT 1
#define ESP_SLEN 2
#define ESP_SSPI 3

#define ESP_MAX_FAIL 15
#define ESP_FAIL 0
#define ESP_OK 1

#define espuart huart1
#define espspi hspi1

uint8_t ESPSend(uint8_t *raw,size_t len);
void ESPInit();
void ESPClear();
uint16_t ESPCalcParity(uint16_t len);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


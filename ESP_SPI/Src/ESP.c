#include "ESP.h"
#include "gpio.h"
#include "spi.h"
#include "dma.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <math.h>

TaskHandle_t ESPTaskHandle = NULL;
uint8_t lraw[4];
uint8_t ESPState = ESP_INIT;
uint8_t clear[] = {ESP_END, ESP_END, ESP_END, ESP_END};
int8_t fails = 0;

uint16_t ESPCalcParity(uint16_t len)
{
    uint8_t odd,even,all;
    odd = even = all = 0;
    for (uint8_t i=0;i<9;i++)
    {
        odd ^= i & 1 & ( (len>>i) & 1);
        even ^= ((i & 1) ^ 1) & ( (len>>i) & 1);
        all ^= ( (len>>i) & 1);
    }
    return (len | (odd << 9) | (even << 10) | (all << 11));
}

uint8_t ESPSend(uint8_t *raw,size_t len)
{
    ESPTaskHandle = xTaskGetCurrentTaskHandle();
    len = ESPCalcParity(len);
    *((uint16_t *)lraw) = len;
    ESPState = ESP_SLEN;
    HAL_UART_Transmit(&espuart,lraw,4,300);
    itm_printf("UART?\n");
    if (ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(800)))
        return ESPClear(),ESP_FAIL;
    else 
        fails = MAX(0,fails-2);
    itm_printf("UART!\n");
    ESPState = ESP_SSPI;
    HAL_SPI_Transmit_DMA(&espspi,raw,len);
    HAL_GPIO_WritePin(ESP_notify_GPIO_Port,ESP_notify_Pin,GPIO_PIN_SET);
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    HAL_GPIO_WritePin(ESP_notify_GPIO_Port,ESP_notify_Pin,GPIO_PIN_RESET);
    ESPTaskHandle = NULL;
    return ESP_OK;
}

void ESPClear()
{
    if (++fails > ESP_MAX_FAIL)
        ESPInit();
    HAL_UART_Transmit(&espuart,clear,sizeof(clear),500);
}

void ESPInit()
{
    lraw[2] = lraw[3] = ESP_END;
    ESPState = ESP_INIT;
    HAL_GPIO_WritePin(ESP_reset_GPIO_Port,ESP_reset_Pin,GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ESP_reset_GPIO_Port,ESP_reset_Pin,GPIO_PIN_SET);
    while (HAL_GPIO_ReadPin(ESP_handshake_GPIO_Port,ESP_handshake_Pin)==GPIO_PIN_RESET);
    ESPState = ESP_IDLE;
    return ;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t Woken = 0;
    if (GPIO_Pin == ESP_handshake_Pin && ESPState != ESP_INIT)
    {
        if (ESPTaskHandle == NULL)
            itm_printf("What the fuck?ESP_handshake pull up without any transmission\n");
        else if (HAL_GPIO_ReadPin(ESP_handshake_GPIO_Port,ESP_handshake_Pin) == GPIO_PIN_RESET)
            if (ESPState != ESP_SLEN)
                itm_printf("Strange, ESPTaskHandle trigger HIGH without ESP_SLEN");
            else
                vTaskNotifyGiveFromISR(ESPTaskHandle,&Woken);
        else if (HAL_GPIO_ReadPin(ESP_handshake_GPIO_Port,ESP_handshake_Pin) == GPIO_PIN_SET)
            if (ESPState != ESP_SSPI)
                itm_printf("Strange, ESPTaskHandle trigger LOW without ESP_SSPI");
            else
                vTaskNotifyGiveFromISR(ESPTaskHandle,&Woken);
        else
            itm_printf("How?\n");
    }
    if (ESPState != ESP_INIT)
        portYIELD_FROM_ISR(Woken);
    HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
}
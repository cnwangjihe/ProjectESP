/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ESP.h"
#include "retarget.h"
#include "message_buffer.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t raw[600];
MessageBufferHandle_t ReceiveQueueHandle;
/* USER CODE END Variables */
osThreadId MainTaskHandle;
osThreadId UDPReceiveHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartMainTask(void const * argument);
void StartUDPReceive(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  RetargetInit();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  ReceiveQueueHandle = xMessageBufferCreate(UART_BUF_SIZE * 8);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of MainTask */
  osThreadDef(MainTask, StartMainTask, osPriorityNormal, 0, 128);
  MainTaskHandle = osThreadCreate(osThread(MainTask), NULL);

  /* definition and creation of UDPReceive */
  osThreadDef(UDPReceive, StartUDPReceive, osPriorityAboveNormal, 0, 128);
  UDPReceiveHandle = osThreadCreate(osThread(UDPReceive), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartMainTask */
/**
  * @brief  Function implementing the MainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void const * argument)
{
  /* USER CODE BEGIN StartMainTask */
  /* Infinite loop */
  // uint8_t raw[] = {'a','b','c','d','e','f','g','h','i','j'};
  uint16_t count = 0;
  uint16_t cnt = 0;
  ESPInit();
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  memset(raw,0xEE,sizeof(raw));
  raw[0]='\x01';
  raw[1]='\x02';
  itm_printf("^Start^\n");
  for(;;)
  {
    ESPSend(raw,506);
    if (++count == 20720)
    {
      itm_printf("^Got%d^\n",cnt++);
      count = 0;
    }
    // osDelay(1);
  }
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartUDPReceive */
/**
* @brief Function implementing the UDPReceive thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUDPReceive */
void StartUDPReceive(void const * argument)
{
  /* USER CODE BEGIN StartUDPReceive */
  /* Infinite loop */
  uint8_t *raw = pvPortMalloc(UART_BUF_SIZE);
  size_t rlen;
  for(;;)
  {
    rlen = xMessageBufferReceive(ReceiveQueueHandle,raw,UART_BUF_SIZE,portMAX_DELAY);
    itm_printf("Got:#%.*s#\n",rlen,raw);
  }
  /* USER CODE END StartUDPReceive */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

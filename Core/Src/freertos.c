/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "base.h"
#include "ai.h"
#include <limits.h>

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
QueueHandle_t dataQueue = NULL;
QueueHandle_t dataLenQueue = NULL;

static unsigned char recvBuffer[MAX_RECV_BUFFER] = {0};  // 接收缓冲�??????
uint8_t sendBuffer[MAX_RECV_BUFFER] = {0};  // 发�?�缓冲区
// 事件组可以代替信号量的工作，完成任务与任务，中断与任务的通信
EventGroupHandle_t EventGroup;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId DataRecvHandle;
osThreadId LCDConrtolHandle;
osThreadId DataProcessHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartDataRecv(void const * argument);
void StartLCDCTL(void const * argument);
void StartDataProcess(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  EventGroup = xEventGroupCreate();

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  dataQueue = xQueueCreate(QUEUE_MAX_SIZE, sizeof(unsigned char));
  dataLenQueue = xQueueCreate(LEN_QUEUE_MAX_SIZE, sizeof(uint16_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of DataRecv */
  osThreadDef(DataRecv, StartDataRecv, osPriorityHigh, 0, 128);
  DataRecvHandle = osThreadCreate(osThread(DataRecv), NULL);

  /* definition and creation of LCDConrtol */
  osThreadDef(LCDConrtol, StartLCDCTL, osPriorityNormal, 0, 128);
  LCDConrtolHandle = osThreadCreate(osThread(LCDConrtol), NULL);

  /* definition and creation of DataProcess */
  osThreadDef(DataProcess, StartDataProcess, osPriorityHigh, 0, 128);
  DataProcessHandle = osThreadCreate(osThread(DataProcess), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for (;;) {
    osDelay(10000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartDataRecv */
/**
* @brief Function implementing the DataRecv thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataRecv */
void StartDataRecv(void const * argument)
{
  /* USER CODE BEGIN StartDataRecv */
  printf("Data recv task created \r\n");

  uint16_t dataLen = 0;

  /* Infinite loop */
  while (1) {
    if (pdTRUE!= xQueueReceive(dataLenQueue, &dataLen, portMAX_DELAY)) {
      continue;
    }
		
    if (dataLen >= MAX_RECV_BUFFER - 2) {
      printf("Queue len is too long, resetting\r\n");
      dataLen = 0;
      memset(recvBuffer, 0, MAX_RECV_BUFFER);
      continue;
    }

    // 接受dataLen长度的数�?????
    for (int i = 0; i < dataLen; i++) {
      if (pdTRUE!= xQueueReceive(dataQueue, &recvBuffer[i], portMAX_DELAY)) {
        printf("Queue receive failed\r\n");
        break;
      }
    }
    
    void* unpacked_data = NULL;
    CommuDataType unpacked_type;

    if (PackageManager_unpackage((const PackageManager*)package_manager, recvBuffer, dataLen, &unpacked_data, &unpacked_type) == 0) {
      printf("Failed to unpack data\r\n");
      continue;
    }

    switch (unpacked_type) {
      case TELEMETRY: {
        TelemetryStruct* telemetry = (TelemetryStruct*)unpacked_data;
        xTaskNotify(DataProcessHandle, (uint32_t)telemetry, eSetValueWithOverwrite);
        break;
      }	
      default: {
        printf("Unknown data type: %d\r\n", unpacked_type);
        break;	
      }
    }

    if (unpacked_data != NULL) {
        free(unpacked_data); // 释放内存 
    }
        
    dataLen = 0;
    memset(recvBuffer, 0, MAX_RECV_BUFFER);
  }
	
  /* USER CODE END StartDataRecv */
}

/* USER CODE BEGIN Header_StartLCDCTL */
/**
 * @brief Function implementing the LCDConrtol thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartLCDCTL */
void StartLCDCTL(void const * argument)
{
  /* USER CODE BEGIN StartLCDCTL */
  printf("LCD control task created \r\n");
  int k = 0;
  char showData[30] = {0};
  
  /* Infinite loop */
  while (1) {
    EventBits_t bits = xEventGroupWaitBits(
        EventGroup,
        0x01,
        pdTRUE,      
        pdFALSE,
        portMAX_DELAY
    );
    
    if ((bits & 0x01) != 0) {
      sprintf(showData, "k: %d", ++k);
      LCD_ShowString(20, 80, 210, 24, 24, (u8 *)showData);
    }
  }
  /* USER CODE END StartLCDCTL */
}

/* USER CODE BEGIN Header_StartDataProcess */
/**
* @brief Function implementing the DataProcess thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataProcess */
void StartDataProcess(void const * argument)
{
  /* USER CODE BEGIN StartDataProcess */
	printf("Data process task created \r\n");
  
  uint32_t notificationValue = 0;
  TelemetryStruct* pTelemetry = NULL;
  /* Infinite loop */
  while(1) {
    if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, portMAX_DELAY) == pdTRUE) {
      pTelemetry = (TelemetryStruct*)notificationValue;
      if (pTelemetry == NULL) {
        printf("E: Failed to get telemetry data\r\n");
        continue;
      }

      if (AI_Run_Inference(pTelemetry) != 0) {
        printf("E: AI_Run_Inference failed\r\n");
        continue;
      }

      if (m_ai_output == NULL || m_ai_output[0].data == NULL) {
        printf("错误：AI 输出未初始化！\n");
        return;
      }

      // 2. 直接读取输出数据（假设输出是 float[3]�??
      float* output_data = (float*)m_ai_output[0].data;
      pTelemetry->tx = output_data[0];
      pTelemetry->ty = output_data[1];
      pTelemetry->tz = output_data[2];

      uint16_t telemetry_package_len = calculate_package_length((const PackageManager*)package_manager, TELEMETRY, NO_FAULT);
      PackageManager_package((const PackageManager*)package_manager, (const void*)pTelemetry, TELEMETRY, sendBuffer);
      HAL_UART_Transmit_DMA(&huart1, sendBuffer, telemetry_package_len);
      xEventGroupSetBits(EventGroup, 0x01);
      
			if (pTelemetry != NULL) {
				free(pTelemetry);
			}
			
    }
  }
  /* USER CODE END StartDataProcess */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/* USER CODE END Application */

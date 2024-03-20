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
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_QUEUE 100
#define MAX_SEM_COUNT 100
#define MAX_BUFFER_SIZE 0x3FFF
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
typedef struct recvStruct {
  uint16_t index;
  unsigned char buffer[MAX_BUFFER_SIZE];
  bool frameHeadMatched;
  bool dataRecvFinish;
  int frameTailCount;
} recvStruct;

QueueHandle_t dataTransQueue = NULL;
recvStruct recvData = {.index = 0, .buffer = {0}};

// 事件组可以代替信号量的工作，完成任务与任务，中断与任务的通信
EventGroupHandle_t EventGroup;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId DataTransHandle;
osThreadId LCDConrtolHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
bool isFrameHead(char c, int index, const char *head);
void clearStruct(recvStruct *r);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);
void StartDataTrans(void const *argument);
void StartLCDCTL(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

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
  dataTransQueue = xQueueCreate(MAX_QUEUE, sizeof(unsigned char));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of DataTrans */
  osThreadDef(DataTrans, StartDataTrans, osPriorityHigh, 0, 128);
  DataTransHandle = osThreadCreate(osThread(DataTrans), NULL);

  /* definition and creation of LCDConrtol */
  osThreadDef(LCDConrtol, StartLCDCTL, osPriorityNormal, 0, 128);
  LCDConrtolHandle = osThreadCreate(osThread(LCDConrtol), NULL);

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
void StartDefaultTask(void const *argument) {
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for (;;) {
    osDelay(10000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartDataTrans */
/**
 * @brief Function implementing the dataTrans thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDataTrans */
void StartDataTrans(void const *argument) {
  /* USER CODE BEGIN StartDataTrans */
  unsigned char outData2PC[8 + 8];
  unsigned char recvCharTmp;

  printf("Data trans task created \r\n");
  /* Infinite loop */

  while (1) {
    osDelay(10000);
    if (pdTRUE != xQueueReceive(dataTransQueue, &recvCharTmp, portMAX_DELAY))
      continue;

    printf("data trans task recv: %c \r\n", recvCharTmp);
    if (!recvData.dataRecvFinish) // recv not finish
    {
      if (recvData.frameHeadMatched) {
        // start recv
        if (recvCharTmp != 'E') // char 'e' is not the end flag, continue recv
          recvData.frameTailCount = 0;
        else
          recvData.frameTailCount++;

        recvData.buffer[recvData.index & MAX_BUFFER_SIZE] = recvCharTmp;
        recvData.index++;

        if (recvData.frameTailCount == 8) {
          recvData.frameTailCount = 0;
          recvData.dataRecvFinish = true; // recv finish
        }

        if (recvData.index > MAX_BUFFER_SIZE) {
          clearStruct(&recvData);
          continue;
        }
      } else {
        // match the frame head
        if (isFrameHead(recvCharTmp, recvData.index, "STARTTTT")) {
          recvData.buffer[recvData.index & MAX_BUFFER_SIZE] = recvCharTmp;
          recvData.index++;
          if (recvData.index == 8)
            recvData.frameHeadMatched = true;
        } else if (recvData.index < 8) {
          clearStruct(&recvData);
        }
      }
    }

    if (recvData.dataRecvFinish) // recv finish, and then start processing
    {
      // data frame head
      outData2PC[0] = 'S';
      outData2PC[1] = 'A';
      outData2PC[2] = 'A';
      outData2PC[3] = 'A';
      outData2PC[4] = 'A';
      outData2PC[5] = 'A';
      outData2PC[6] = 'A';
      outData2PC[7] = 'A';
      // data frame tail
      unsigned char endByte[8];
      endByte[0] = 'E';
      endByte[1] = 'E';
      endByte[2] = 'E';
      endByte[3] = 'E';
      endByte[4] = 'E';
      endByte[5] = 'E';
      endByte[6] = 'E';
      endByte[7] = 'E';

      memcpy(outData2PC + 8, endByte, sizeof(endByte));
      clearStruct(&recvData);

      // event group method
      // xEventGroupSetBits(EventGroup, 0x1);

      // 任务通知
      xTaskNotify(LCDConrtolHandle, 1, eSetBits);
    }
  }
  /* USER CODE END StartDataTrans */
}

/* USER CODE BEGIN Header_StartLCDCTL */
/**
 * @brief Function implementing the LCDConrtol thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartLCDCTL */
void StartLCDCTL(void const *argument) {
  /* USER CODE BEGIN StartLCDCTL */
  printf("LCD control task created \r\n");
  int k = 0;
  char showData[30] = {0};
  uint32_t bits = 0;
  /* Infinite loop */
  while (1) {
    osDelay(10000);
    // event group method
    // EventBits_t bits =
    //     xEventGroupWaitBits(EventGroup, 0x1, pdTRUE, pdTRUE, portMAX_DELAY);
    // if ((bits & (0x01)) == (0x01)) {
    //   // update LCD
    //   sprintf(showData, "k: %d", ++k);
    //   LCD_ShowString(20, 80, 210, 24, 24, (u8 *)showData);
    // }

    // 任务通知
    if (xTaskNotifyWait(0, 0, &bits, portMAX_DELAY)) {
      if (bits == 0x01) {
        // update LCD
        sprintf(showData, "k: %d", ++k);
        LCD_ShowString(20, 80, 210, 24, 24, (u8 *)showData);
      }
    }
  }
  /* USER CODE END StartLCDCTL */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

bool isFrameHead(char c, int index, const char *head) {
  if (index >= 8)
    return false;
  return c == head[index];
}

void clearStruct(recvStruct *r) {
  r->index = 0;
  memset(r->buffer, 0, sizeof(r->buffer));
  r->frameHeadMatched = false;
  r->dataRecvFinish = false;
  r->frameTailCount = 0;
}

/* USER CODE END Application */

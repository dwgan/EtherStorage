/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
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

#include "stdio.h"
#include "tcp_server.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LAN8720_PHY_ADDRESS  0x00  // 根据你的硬件设计设置正确的PHY地址
//#define PHY_BSR              0x01  // LAN8720的基本状态寄存器地址
    
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId ethernetTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern struct netif gnetif;
extern ETH_HandleTypeDef heth;

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void EthernetTask(void const * argument);

extern void MX_LWIP_Init(void);
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

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of ethernetTask */
  osThreadDef(ethernetTask, EthernetTask, osPriorityNormal, 0, 256);
  ethernetTaskHandle = osThreadCreate(osThread(ethernetTask), NULL);

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
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    char taskListBuffer[512];
    vTaskList(taskListBuffer);
    printf("\r\nTask List:\n%s\r\n\n", taskListBuffer);
    printf("LWIP initialized, IP address: %s\r\n", ip4addr_ntoa(&gnetif.ip_addr));
    
    uint32_t phyreg;      
    if (HAL_ETH_ReadPHYRegister(&heth, LAN8720_PHY_ADDRESS, PHY_BSR, &phyreg) != HAL_OK) {
      printf("Failed to read PHY register\n");
    } else {
      printf("PHY BSR register value: 0x%08X\n", phyreg);
    }
    osDelay(5000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_EthernetTask */
/**
* @brief Function implementing the ethernetTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_EthernetTask */
void EthernetTask(void const * argument)
{
  /* USER CODE BEGIN EthernetTask */
  int sock;
  struct sockaddr_in serverAddr, clientAddr;
  int addr_len = sizeof(clientAddr);
  char recvBuffer[128];
  
  // 创建套接字
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      // 处理错误
      osThreadTerminate(NULL);
  }

  // 配置服务器地址
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(5000);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  // 绑定套接字
  if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
      // 处理错误
      close(sock);
      osThreadTerminate(NULL);
  }

  // 监听连接
  if (listen(sock, 1) < 0) {
      // 处理错误
      close(sock);
      osThreadTerminate(NULL);
  }

  for(;;)
  {
    int newSock = accept(sock, (struct sockaddr *)&clientAddr, (socklen_t *)&addr_len);
    if (newSock < 0) {
        // 处理错误
        continue;
    }

    while (1) {
        int recvLen = recv(newSock, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (recvLen > 0) {
            recvBuffer[recvLen] = '\0';
            HAL_UART_Transmit(&huart1, (uint8_t*)recvBuffer, recvLen, HAL_MAX_DELAY);
        } else {
            // 处理连接断开
            break;
        }
    }
    close(newSock);
  }

  /* USER CODE END EthernetTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

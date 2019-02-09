/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "ringbuffer.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
uint8_t byte;
RingBuffer  mbRxRingBuf;
uint8_t     mbRxBuf[200];
uint16_t MBResponseTimeout = 300;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
void RS485_preTransmission(void);
void RS485_postTransmission(void);
static inline uint8_t lowByte(uint16_t);
static inline uint8_t highByte(uint16_t);
uint16_t CRC16(uint8_t *, uint16_t);
void debbugXui(char*);//uint8_t*);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t CRC16(uint8_t *pucData, uint16_t usLen) {
	uint16_t usResult, usI, usJ;

	usResult = 0xFFFF;
	usI = 0;

	while (usI < usLen) {
		usResult ^= (uint16_t)*(pucData + usI);

		for (usJ = 0; usJ < 8; usJ++) {
			if (usResult & 0x0001) {
				// ????? ?????? CARRY ????? 1
				usResult	= usResult >> 1;
				usResult	^= 0xA001;
				
			} else {
				// ????? ?????? CARRY ????? 1
				usResult	= usResult >> 1;
			}
		}
		usI++;
	}
	
	return usResult;
}

void ModBusRead(uint16_t adr, uint16_t len) {
	//uint8_t buf[8]    = {	0x01, 					0x03, 		adr / 256, adr % 256, 	len / 256, len % 256,			0x00, 0x00};
	
	//uint8_t bufRESET[8] = {0x1, 0x06, highByte(0x2199), lowByte(0x2199), highByte(0x8D), lowByte(0x8D), 0x00, 0x00};
	//uint16_t rez2	= CRC16(bufRESET, 6);
	//bufRESET[6] = rez2 % 256;
	//bufRESET[7] = rez2 / 256; 
	//HAL_UART_Transmit(&huart2, bufRESET, 8, 0x100);
	//uint8_t rez2 = ("%d") len;
	
	uint8_t buf[8]    = {0x01, 0x03, highByte(adr), lowByte(adr), highByte(len), lowByte(len),	0x00, 0x00};
	uint8_t buf2[25]; 
	uint16_t rez	= CRC16(buf, 6);
	buf[6] = rez % 256;
	buf[7] = rez / 256;
  uint8_t xresult;
	uint8_t xRespBtCnt = len*2 + 5;
	uint8_t mbADUSize = 0;
	RS485_preTransmission();
	
	if (HAL_UART_Transmit(&huart2, buf, 8, 0x100) != HAL_OK) return;
	
	RS485_postTransmission();
  uint32_t u32StartTime = Modbus_Master_Millis();
	uint8_t TimeOUTState = 0;
	while(xRespBtCnt != 0 && !TimeOUTState){
	//while(!TimeOUTState){
	//	xresult = HAL_UART_Receive(&huart2, buf2, xRespBtCnt, 0x100);
 	//xresult = HAL_UART_Receive(&huart2, &byte, 1, 0x100);
		xresult = Modbus_Master_Rece_Handler();//(&xRespBtCnt);
		if(xresult == HAL_OK){
 			xRespBtCnt--;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
			if (Modbus_Master_Rece_Available())
			{
				buf2[mbADUSize++] = Modbus_Master_Read();
				//u8BytesLeft--;
			}
      //rbPush(&mbRxRingBuf, (uint8_t)(byte & (uint8_t)0xFFU));	
		}
		else if(xresult == HAL_ERROR){
			debbugXui("ERR");
			//HAL_Delay(100);
		}
		if ((Modbus_Master_Millis() - u32StartTime) > MBResponseTimeout)
    {
			TimeOUTState = 1;
			debbugXui("TIM");
      //u8MBStatus = ku8MBResponseTimedOut;//226
    }
	}
	if(xRespBtCnt == 0){
		debbugXui("KYL");
	}
	else debbugXui("BAD");
}


void debbugXui(char* receiveVal){//(uint8_t* receiveVal){
	char str[30];		
	sprintf(str,"%s",receiveVal);
	//sprintf(str+2,"%x",buf2[4]);
			//sprintf(str+3,"%x",buf2[5]);
			//sprintf(str+4,"%x",buf2[6]);
			//sprintf(str+6,"%x",buf2[7]);
			//sprintf(str+7,"%x",buf2[8]);
			//sprintf(str+9,"%x",buf2[9]);
			//sprintf(str+10,"%x",buf2[10]);
			//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	//HAL_Delay(1000);
	HAL_UART_Transmit(&huart1, (uint8_t*)receiveVal, sizeof(receiveVal), 0x100);
	HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 0x100);
}
/*
static inline uint8_t lowByte(uint16_t ww)
{
  return (uint8_t) ((ww) & 0x00FF);
}

static inline uint8_t highByte(uint16_t ww)
{
  return (uint8_t) ((ww) >> 8);
}*/
void RS485_preTransmission() {
    HAL_GPIO_WritePin(MB_USART2_TXEN_GPIO_Port, MB_USART2_TXEN_Pin, GPIO_PIN_SET);
    //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);// TODO: :??? ???????, ?????? !!!
}

void RS485_postTransmission() {
    HAL_GPIO_WritePin(MB_USART2_TXEN_GPIO_Port, MB_USART2_TXEN_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);// TODO: :??? ???????, ?????? !!!
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	rbInitialize(&mbRxRingBuf, mbRxBuf, sizeof(mbRxBuf));
	//HAL_UART_Receive(&huart1, &byte, 1, 0x100);
  //uint8_t  byt[10] = "hui";
	//byte = "hu";
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		ModbusMaster_writeSingleRegister(0x1, 0x2199,0x8D);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		ModBusRead(0x0C1E, 4);//0x0004);
		HAL_Delay(100);
		/*
		RS485_preTransmission();
		HAL_UART_Transmit(&huart2, &byte, 1, 0x1000 );
    RS485_postTransmission();
		
		HAL_Delay(100);
		
		HAL_UART_Receive(&huart2, &byte, 1, 0x100);
    */
		/* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 19200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 19200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MB_USART2_TXEN_GPIO_Port, MB_USART2_TXEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : MB_USART2_TXEN_Pin */
  GPIO_InitStruct.Pin = MB_USART2_TXEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(MB_USART2_TXEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */


/* This callback is called by the HAL_UART_IRQHandler when the given number of bytes are received */


//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
	//huart->Instance
  //if (huart->Instance == USART2)
  //{
    /* Transmit one byte with 100 ms timeout */
    //HAL_UART_Transmit(&huart2, &byte, 1, 100);

    /* Receive one byte in interrupt mode */ 
    //HAL_UART_Receive_IT(&huart2, &byte, 1);
  //}
//}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

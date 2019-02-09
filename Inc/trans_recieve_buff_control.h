/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TANS_RECEIEVE_BUFF_CONTROL_H
#define __TANS_RECEIEVE_BUFF_CONTROL_H
  /* Includes ------------------------------------------------------------------*/
//�������йص�ͷ�ļ�
#include "bsp_board.h"
/* USER CODE BEGIN Includes */
//modbus���������ʹ�õ���ͷ�ļ�

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

uint8_t Modbus_Master_RB_Initialize(void);
uint8_t Modbus_Master_Rece_Flush(void);
uint8_t Modbus_Master_Rece_Available(void);
uint8_t Modbus_Master_GetByte(uint8_t  *getbyte);
uint8_t Modbus_Master_GetByte_Size(uint8_t* getbyte, uint8_t* aduSize);
uint8_t Modbus_Master_Rece_Handler(void);
uint8_t Modbus_Master_Receive_Handler(uint8_t* aduSize);
uint8_t Modbus_Master_Read(void);
uint8_t Modbus_Master_Write(uint8_t *buf,uint8_t length);
uint32_t Modbus_Master_Millis(void);
#endif 
/********END OF FILE****/

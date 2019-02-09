#include "trans_recieve_buff_control.h"
#include "ringbuffer.h"

RingBuffer  m_Modbus_Master_RX_RingBuff;
uint8_t     m_Modbus_Master_RX_Buff[200];
uint8_t			Fcounter = 0;
uint8_t 		FRECbuff[100];


extern UART_HandleTypeDef huart2;
/**
  * @brief  Инициализирует конфигурацию кольцевой очереди кольцевого буфера, полученную прерыванием.Байты, полученные прерыванием, управляются с помощью m_Modbus_Master_RX_RingBuff.
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_RB_Initialize(void)
{
	/* Инициализация конфигурации кольцевого буфера */
	rbInitialize(&m_Modbus_Master_RX_RingBuff, m_Modbus_Master_RX_Buff, sizeof(m_Modbus_Master_RX_Buff));
	return 1 ;
}


/**
  * @brief  короткая очередь звонков
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_Rece_Flush(void)
{
  rbClear(&m_Modbus_Master_RX_RingBuff);
}
/**
  * @brief   определяет, есть ли необработанные байты в кольцевом буфере GPS
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_Rece_Available(void)
{
	/* Если буфер пакетов переполнен, очистить его, пересчитать */
		if(m_Modbus_Master_RX_RingBuff.flagOverflow==1)
		{
			rbClear(&m_Modbus_Master_RX_RingBuff);
		}
	return !rbIsEmpty(&m_Modbus_Master_RX_RingBuff);
}

/****************************************************************************************************/
/*Ниже приведена часть, связанная с уровнем аппаратного интерфейса, который трансплантируется в соответствии со способом обработки различных процессоровֲ*/
/**
  * @brief  получает значение в регистре получения
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_GetByte(uint8_t  *getbyte)
{
  if(HAL_UART_Receive (&huart2 ,(uint8_t *)getbyte,1,0x100) != HAL_OK)//(&huart2 ,(uint8_t *)getbyte,1,0x01) != HAL_OK) //(&huart2, getbyte, 1U) == HAL_OK)
  {
		return HAL_ERROR;
	}
	else
	{
	  return HAL_OK;
	}
}

uint8_t Modbus_Master_GetByte_Size(uint8_t  *getbyte, uint8_t* aduSize)
{
  if(HAL_UART_Receive (&huart2 ,getbyte,*aduSize,0x1000) != HAL_OK)//(&huart2 ,(uint8_t *)getbyte,1,0x01) != HAL_OK) //(&huart2, getbyte, 1U) == HAL_OK)
  {
		return HAL_ERROR;
	}
	else
	{
	  return HAL_OK;
	}
}

/**
  * @brief  короткий обработчик прерываний, вызываемый в последовательном приемном прерывании: введите значение регистра в буфер
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_Rece_Handler(void)
{
	 uint8_t res,byte ; 
	 
	// Читаем данные внутри регистра и помещаем данные в круговую очередь
   res = Modbus_Master_GetByte(&byte);
	 if(res == HAL_OK)
	 {
		 FRECbuff[Fcounter++] = byte;//TODO понять почему две единицы и вывести тмп бафер в ят.
		 //Fcounter++;
	   if(Fcounter>12){
				Fcounter = 0;
				memset(FRECbuff,0x0,100);
			  HAL_UART_AbortReceive(&huart2);
		 }
		 //rbPush(&m_Modbus_Master_RX_RingBuff, (uint8_t)(byte & (uint8_t)0xFFU));
	 }
	 return res;
}

uint8_t Modbus_Master_Receive_Handler(uint8_t* aduSize)
{
	 uint8_t res,byte[*aduSize];
	// Читаем данные внутри регистра и помещаем данные в круговую очередь
   res = Modbus_Master_GetByte_Size(&byte[0], aduSize);
	 if(res == HAL_OK)
	 {
		 for(uint8_t i = 0; i < *aduSize; i++){
				rbPush(&m_Modbus_Master_RX_RingBuff, (uint8_t)(byte[i] & (uint8_t)0xFFU));
		 }
	 }
	 return res;
}
/**
  * @brief  краткое чтение данных буфера
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_Read(void)
{
	uint8_t cur =0xff;
	if( !rbIsEmpty(&m_Modbus_Master_RX_RingBuff))
	{
		  cur = rbPop(&m_Modbus_Master_RX_RingBuff);
	}
	return cur;
}

/**
  * @brief  отправляет пакет
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint8_t Modbus_Master_Write(uint8_t *buf,uint8_t length)
{
 if(HAL_UART_Transmit(&huart2 ,(uint8_t *)buf,length,0xFFFF))
 {
   return HAL_ERROR;
 }
 	else
	{
	  return HAL_OK;
	}
}
/*
 if(HAL_UART_Transmit(&huart2 ,(uint8_t *)buf,length,0xff))
 {
   return HAL_ERROR;
 }
 	else
	{
	  return HAL_OK;
	}
}*/

/**
  * @brief  краткий таймер цикла 1 мс
  * @param
  * @note
  * @retval void
  * @author xiaodaqi
  */
uint32_t Modbus_Master_Millis(void)
{
  return HAL_GetTick();
}

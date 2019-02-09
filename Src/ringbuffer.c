#include "ringbuffer.h"
/**
  * @brief  rbInitialize Инициализировать конфигурацию, заполнить буфер информацией в структуре
  * @param  pRingBuff:ringbuffer структурa
	*         buff Буфер данных
	*         length Размер буфера
  * @note   
  * @retval void
  * @author xiaodaqi
  */	
void rbInitialize(RingBuffer* pRingBuff, uint8_t* buff, uint16_t length)
{
	pRingBuff->pBuff = buff;
	pRingBuff->pEnd  = buff + length;
	pRingBuff->wp = buff;
	pRingBuff->rp = buff;
	pRingBuff->length = length;
	pRingBuff->flagOverflow = 0;
}

/**
  * @brief  удаление ringbuffer Информация о структуре
  * @param  pRingBuff в ожидании ringbuffer
  * @note   
  * @retval void
  * @author xiaodaqi
  */	

 void rbClear(RingBuffer* pRingBuff)
{
 	pRingBuff->wp = pRingBuff->pBuff;
	pRingBuff->rp = pRingBuff->pBuff;
	pRingBuff->flagOverflow = 0;
}

/**
  * @brief  Вставьте один байт в буфер
  * @param  pRingBuff в ожидании ringbuffer
  *         value Прессованные данные
  * @note   
  * @retval void
  * @author xiaodaqi
  */	
 void rbPush(RingBuffer* pRingBuff, uint8_t value)
{
	uint8_t* wp_next = pRingBuff->wp + 1;
	if( wp_next == pRingBuff->pEnd ) {
		wp_next -= pRingBuff->length; // Rewind pointer when exceeds bound Перематывать указатель при превышении границы
	}
	if( wp_next != pRingBuff->rp ) {
		*pRingBuff->wp = value;
		pRingBuff->wp = wp_next;
	} else {
		pRingBuff->flagOverflow = 1;
	}
}

/**
  * @brief  Выдвиньте один байт в буфер
  * @param  pRingBuff в ожидании ringbuffer   
  * @note   
  * @retval Прессованные данные
  * @author xiaodaqi
  */	
 uint8_t rbPop(RingBuffer* pRingBuff)
{
	if( pRingBuff->rp == pRingBuff->wp ) return 0; // empty
  
	uint8_t ret = *(pRingBuff->rp++);
	if( pRingBuff->rp == pRingBuff->pEnd ) {
		pRingBuff->rp -= pRingBuff->length; // Rewind pointer when exceeds bound
	}
	return ret;
}

/**
  * @brief  Получить количество байтов, которые не были обработаны буфером
  * @param  pRingBuff в ожидании ringbuffer   
  * @note   
  * @retval Количество байтов для обработки
  * @author xiaodaqi
  */
 uint16_t rbGetCount(const RingBuffer* pRingBuff)
{
	return (pRingBuff->wp - pRingBuff->rp + pRingBuff->length) % pRingBuff->length;
}

/**
  * @brief  Определите, пуст ли буфер
  * @param  pRingBuff���������ringbuffer   
  * @note   
  * @retval Пусто равно 1, иначе 0
  * @author xiaodaqi
  */
 int8_t rbIsEmpty(const RingBuffer* pRingBuff)
{
	return pRingBuff->wp == pRingBuff->rp; 
}

/**
  * @brief Проверка на фулл
  * @param  pRingBuff���������ringbuffer   
  * @note   
  * @retval Полный равен 1, в противном случае 0
  * @author xiaodaqi
  */
 int8_t rbIsFull(const RingBuffer* pRingBuff)
{
 	return (pRingBuff->rp - pRingBuff->wp + pRingBuff->length - 1) % pRingBuff->length == 0;
}

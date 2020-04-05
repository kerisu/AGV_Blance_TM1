#ifndef __COMHANDLE_H
#define __COMHANDLE_H

#include "stm32f4xx.h"
typedef struct
{
	uint8_t Buf[100];	//���ڽ��ջ�����
	uint8_t Over;			//���ڽ��ܼ��
	uint8_t Len;				//���ڽ��ܳ���
}UartBufTypedef;

//�����ֽ���д�뵽������
void UsartRxToBuf(uint8_t data);
//ͨ�Ŵ���ѭ��
void comHandle_Loop(void);
//Usart2 data transmit
void Usart2Tx(uint8_t * data,uint8_t len);
//Usart2 Str transmit
void Usart2TxStr(uint8_t * data);


#endif /*__COMHANDLE_H*/


/**
  ******************************************************************************
  * @file    comHandle.c
  * @author  �Ŷ�
  * @version V1.0.0
  * @date    2019-12-16
  * @brief   ͨ�Ŵ�����
  ******************************************************************************
  */
#include "comHandle.h"
#include "usart.h"
#include "pidctrller.h"
#include "stdio.h"

#define UART_COM_STAT   '<'	//����ͨ�ſ�ʼ�ֽ�
#define UART_COM_END		'>'	//����ͨ�Ž����ֽ�

UartBufTypedef g_uartComData = {.Over = 0 ,.Len = 0};//���ڽ��ջ�����

//�����ֽ���д�뵽������
void UsartRxToBuf(uint8_t data)
{
	//��һ����Ϣ�������
	if(g_uartComData.Over == 0)
	{
		//��ʼ
		if(data == UART_COM_STAT )
		{
			//��������
			g_uartComData.Len = 0;
		}
		//����
		else if(data == UART_COM_END)
		{
			//���ս���
			g_uartComData.Over = 1;
		}
		//����
		else
		{
			//д������
			g_uartComData.Buf[g_uartComData.Len] = data;
			//�ƶ����
			g_uartComData.Len = (g_uartComData.Len + 1) % 100;
		}
	}
}
//Usart2 data transmit
void Usart2Tx(uint8_t * data,uint8_t len)
{
	while(len--)
	{
		while((USART2->SR&0X40)==0);	
		USART2->DR = (uint8_t)*data++;
	}
}
//Usart2 Str transmit
void Usart2TxStr(uint8_t * data)
{
	while(*data)
	{
		while((USART2->SR&0X40)==0);	
		USART2->DR = (uint8_t)*data++;
	}
}

/*receive analysis ------------------------------------------------------*/

void MoveCtrl(uint8_t * cmd)
{
	//��������ģʽ
	uint8_t ms1 = cmd[1];
	uint8_t ms2 = cmd[5];
		
	//������������
	uint16_t pwm1 = (cmd[2] - 0x30) * 100+ 
									(cmd[3] - 0x30) * 10+ 
									(cmd[4] - 0x30);
	
	uint16_t pwm2 = (cmd[6] - 0x30) * 100+ 
									(cmd[7] - 0x30) * 10+ 
									(cmd[8] - 0x30);
	
	//�޸Ŀ�����Ŀ����ֵ
	int16_t tg1 = (ms1 == 'A') ? pwm1 : -pwm1;
	int16_t tg2 = (ms2 == 'A') ? pwm2 : -pwm2;
	
	uint8_t str[50];
	sprintf((char *)str,"P1 = %d,P2 = %d\r\n",tg1,tg2);
	Usart2TxStr(str);
	
	SetTargetSpeed(tg1,tg2);
}


void AngelCtrl(uint8_t * cmd)
{
	//��������ģʽ
	uint8_t ms1 = cmd[1];
	
	//������������
	uint16_t spd  = (cmd[2] - 0x30) * 100+ 
									(cmd[3] - 0x30) * 10+ 
									(cmd[4] - 0x30);
	
	//�޸Ŀ�����Ŀ����ֵ
	int16_t tg1 = (ms1 == 'A') ? spd : -spd;
	
	uint8_t str[50];
	sprintf((char *)str,"Spd = %d\r\n",tg1);
	Usart2TxStr(str);
	
	setAngelSpd(tg1);
	
}

//��Ϣ���ս���
void RxBufAnalysis(UartBufTypedef * buf)
{
	//С�����Ա�ʹ��
	if(buf->Over == 1)
	{	
		//ͨ��Э��
		//��ʼ ��<�� ���� '>'
		//[0] ��ʶ [n] ����
		switch(buf->Buf[0])
		{
			case 'M'://������� <MSXXXSXXX> S���� A��B
			{
				MoveCtrl(buf->Buf);
				break;
			}
			case 'V'://������ת���ٶ�
			{
				AngelCtrl(buf->Buf);
				break;
			}
		}
		//���δ��ɱ�־λ
		buf->Over = 0;
	}
}

//ͨ�Ŵ���ѭ��
void comHandle_Loop(void)
{
	//��Ϣ����
	RxBufAnalysis(&g_uartComData);
}




	
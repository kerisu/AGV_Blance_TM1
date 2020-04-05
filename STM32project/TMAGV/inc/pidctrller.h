#ifndef __PIDCTRLLER_H
#define __PIDCTRLLER_H

#include "stm32f4xx.h"

/* Structure type definition -------------------------------------------------*/
typedef struct
{
	float			de ;					//���������ǰ���ֵ difference error
	float			fe ;					//�������������
	float			de1;					//���������ʷ���1
	float			de2;					//���������ʷ���2
	int				out;					//����ͨ������PWM���
	
}PIDDateBaseTypedef;			//pid���ݽṹ����

typedef struct
{
	float			kp;						//����Ȩ��
	float			ki;						//����Ȩ��
	float			kd;						//΢��Ȩ��
	
}PIDParamBaseTypedef;			//pid�����ṹ����

typedef struct
{
	float pitch,roll,yaw; 		//ŷ����
	short aacx,aacy,aacz;			//���ٶȴ�����ԭʼ����
	short gyrox,gyroy,gyroz;	//������ԭʼ����
	short temp;
}IMUDataTypedef;


//�趨�Ƕ�ƫ��
void setAngelOffset(int16_t angel);
//���ý��ٶ�
void setAngelSpd(int16_t spd);
//�趨Ŀ���ٶ�
void SetTargetSpeed(int16_t s1,int16_t s2);
//�趨�������
void setMotorPWM(int16_t s1,int16_t s2);
//�趨ָ����Ч���� ��λ �ٺ���
void setCmdTimeOutVal(uint16_t num);
//pid������ִ�����ڷ�Ƶ��
void PID_Divider(void);

#endif /*__PIDCTRLLER_H*/

#ifndef __FILTER_H
#define __FILTER_H

#include "stm32f4xx.h"

typedef struct
{
 float Input_Butter[3];
 float Output_Butter[3];
}Butter_BufferData;

typedef struct
{
 const float a[3];
 const float b[3];
}Butter_Parameter;

/*һάkalman�˲��ṹ�嶨��,A=1,B=0,H=1*/
typedef struct Kalman_filter
{
	float C_last;				    /*�ϴ�Ԥ�����Э������� C(k|k-1)*/
	float X_last;				    /*ϵͳ״̬Ԥ������о���*/
	
	float Q;						/*��������Э����*/
	float R;						/*��������Э����*/
	
	float K;						/*���������棬�о���*/
	float X;						/*���Ź�����������о���*/
	float C;						/*���Ź���Э�������C(k|k)*/
                            
	float input;				    /*����ֵ����Z(k)*/
}
kal_filter;

//���Իع� ���
void LSMCurveFitting(float * xdata,float * ydata,float * res,uint16_t num);

#endif /*__FILTER_H*/

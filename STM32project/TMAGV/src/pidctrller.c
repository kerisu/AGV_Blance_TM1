/**
  ******************************************************************************
  * @file    pidctrller.c
  * @author  �Ŷ�
  * @version V1.0.0
  * @date    2019-12-17
  * @brief   ƽ�⳵pid
  ******************************************************************************
  */
/*include file ---------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "pidctrller.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "comHandle.h"
#include "usart.h"
#include "stdio.h"
/*Golbal data space ----------------------------------------------------------*/
#define MOTOR_OUTPUT_MAX					999
#define MOTOR_SPEED_PID_KP				10.0
#define MOTOR_SPEED_PID_KI				0.05
#define MOTOR_SPEED_PID_KD				0.000
#define PI												3.1415926535897932384626433832795


IMUDataTypedef g_IMU = 
{
	.pitch = 0, .roll  = 0, .yaw   = 0,
	.aacx  = 0, .aacy  = 0, .aacz  = 0,
	.gyrox = 0, .gyroy = 0, .gyroz = 0,
	.temp  = 0,
};

const PIDParamBaseTypedef PARAM_PID = {//���pid�ٶȿ��Ʋ���

	.kp = MOTOR_SPEED_PID_KP,				//����Ȩ��
	.ki = MOTOR_SPEED_PID_KI,				//����Ȩ��
	.kd = MOTOR_SPEED_PID_KD,				//΢��Ȩ��
};

float 	g_TargetSpeed[2]	 = {0,0};		//Ŀ���ٶ�
float 	g_MotorSpeed[2]		 = {0,0};		//��ǰ�ٶ�
int16_t g_SpdCtrlOut[2]		 = {0,0};		//�ٶȱջ��������
int16_t g_AngelSet				 = 0;				//ƫ�ýǶ�
int16_t g_AngelSpd				 = 0;				//���ý��ٶ�

//�趨�Ƕ�ƫ��
void setAngelOffset(int16_t angel)
{
	g_AngelSet = angel;
}
//���ý��ٶ�
void setAngelSpd(int16_t spd)
{
	g_AngelSpd = spd;
}

//�趨Ŀ���ٶ�
void SetTargetSpeed(int16_t s1,int16_t s2)
{
	g_TargetSpeed[0] = s1;
	g_TargetSpeed[1] = s2;
}
//�ٶ�ֵ����
void updataSpeed(void)
{
	int16_t etr[2];
	
	//ML1 TIM2 MR2 TIM3
	
	//��ñ�����ֵ
	etr[0] = TIM2->CNT;//M1
	TIM2->CNT = 0;
	etr[1] = -TIM3->CNT;//M2
	TIM3->CNT = 0;
	
	for(int i = 0;i < 2;i++)
	{
		float etrtemp = 0;
		
		//��������������ֽ��ٶ�
		//360�ȳ��� 2*80 = 160t
		etrtemp = ((float)(etr[i] % 30000)) / (2.0 * 80.0);
		//���ٶȼ��������ٶ�
		etrtemp = etrtemp * (75 * PI);
		//д�����ٶȵ�ȫ��
		g_MotorSpeed[i] = etrtemp * 10.0 * 0.25;
	}
}

//�趨�������
void setMotorPWM(int16_t s1,int16_t s2)
{
	s2 = 0 - s2;
	//ML1 CH34 MR2 CH12
	if(s1 > 0)
	{
		TIM1->CCR3 = s1;
		TIM1->CCR4 = 0;
	}
	else
	{
		TIM1->CCR3 = 0;
		TIM1->CCR4 = 0 - s1;
	}
	
	if(s2 > 0)
	{
		TIM1->CCR1 = 0;
		TIM1->CCR2 = s2;
	}
	else
	{
		TIM1->CCR1 = 0 - s2;
		TIM1->CCR2 = 0;
	}
}



/**
  * @brief  MotorSpeedPidCtrl.
  * @note		PWM��̬���������㷨��λ��ʽ��ɢ��������΢�ַ��̡�
  *         Out = Kp[e(k)]+ki��e(k)+Kd[e(k)-2e(k-1)+e(k-2)]
  *         ���л������΢�����ܴ��Ŷ���ֵ��Χ��Ч��־λ����
  * @retval None
  */
void MotorSpeedPidCtrl(void)
{
	static PIDDateBaseTypedef pid[2] = {
		[0].de	= 0,	[1].de	= 0,
		[0].fe	= 0,  [1].fe	= 0,
		[0].de1 = 0,  [1].de1 = 0,
		[0].de2 = 0,  [1].de2 = 0,
		[0].out = 0,  [1].out = 0,
	};
	
	//����PID���Ƽ���
	for(int i = 0;i < 2;i++)
	{
		//���㵱ǰ���ƶ���ʷ���
		pid[i].de2	=  pid[i].de1;
		pid[i].de1	=  pid[i].de;
		
		//��������һ�׵�ͨ�˲��� ����0 ��Ŀ���ٶ�
		pid[i].de		=  pid[i].de * 0.7 + 0.3 * (g_MotorSpeed[i] - 0);//g_TargetSpeed[i] - g_MotorSpeed[i];
		pid[i].fe	 +=  pid[i].de - g_TargetSpeed[i] * 10;
		
		//���ַ�������
		if(pid[i].fe > 10000)  pid[i].fe = 10000; 
		if(pid[i].fe <-10000)	 pid[i].fe =-10000; 
		
		
		//����������
		
		//pid[i]���������ķ���
		pid[i].out = 	PARAM_PID.kp * pid[i].de 	+ 
									PARAM_PID.ki * pid[i].fe  + 
									PARAM_PID.kd * ( pid[i].de - 2 * pid[i].de1		+ pid[i].de2) * (pid[i].de < 100);
		//������Ʒ���
		pid[i].out = (pid[i].out > MOTOR_OUTPUT_MAX) ? MOTOR_OUTPUT_MAX : pid[i].out;
		pid[i].out = (pid[i].out <-MOTOR_OUTPUT_MAX) ?-MOTOR_OUTPUT_MAX : pid[i].out;
		
		//�����������
	}
	//������������
	//setMotorPWM(pid[0].out,pid[1].out);
	g_SpdCtrlOut[0] = pid[0].out;
	g_SpdCtrlOut[1] = pid[1].out;
	
	
	//uint8_t str[50];
	//sprintf((char *)str,"%d,%d,%d,%d\r\n",pid[0].out,pid[1].out,pid[2].out,pid[3].out);
	//USART_SendString(USART1,str);
}

//ƽ̨λ�����ݸ���
void AGVImuDataUpdate(void)
{
	//��ȡŷ����
	if(mpu_dmp_get_data(&(g_IMU.pitch),&(g_IMU.roll),&(g_IMU.yaw))==0)
	{		
		g_IMU.temp = MPU_Get_Temperature();					//�õ��¶�ֵ
		MPU_Get_Accelerometer(&(g_IMU.aacx),&(g_IMU.aacy),&(g_IMU.aacz));	//�õ����ٶȴ���������
		MPU_Get_Gyroscope(&(g_IMU.gyrox),&(g_IMU.gyroy),&(g_IMU.gyroz));	//�õ�����������
	}
}


//ƽ̨ƽ�����
void AGVBalancePdCtrl(void)
{
	int16_t vset = (g_SpdCtrlOut[0] + g_SpdCtrlOut[1]) / 2;
	
	float kp = 150.0;
	float kd = 0.15;
	
	//�ǶȻ�
	float post = kp * (g_IMU.roll - g_AngelSet) + kd * g_IMU.gyrox;
	
	if(post > 900)
	{
		post = 900;
	}
	//post = 0;
	
	//ת��
	float kpz = 0.4;
	int 	turn;
	turn = kpz * (g_IMU.gyroz + g_AngelSpd * 10);
	
	//turn = 0;
	//vset = 0;
	
	int16_t out1 = post + vset + turn;
	int16_t out2 = post + vset - turn;
	
	setMotorPWM(out1,out2);
}

//��ӡ������Ϣ
void testMSG(void)
{
  float pitch,roll,yaw; 		//ŷ����
	short aacx,aacy,aacz;			//���ٶȴ�����ԭʼ����
	short gyrox,gyroy,gyroz;	//������ԭʼ����
	short temp = 0;
	
	uint8_t str[50];
	
	//��ȡŷ����
	if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
	{		
		temp=MPU_Get_Temperature();					//�õ��¶�ֵ
		MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//�õ����ٶȴ���������
		MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//�õ�����������
		
		sprintf(str,"pitch = %3.2f,roll = %3.2f,yaw = %3.2f \r\n",pitch,roll,yaw);
		//sprintf(str,"gyrox = %4d,gyroy = %4d,gyroz = %4d \r\n",gyrox,gyroy,gyroz);
		Usart2TxStr(str);
	}
}

//pid������ִ�����ڷ�Ƶ��
void PID_Divider(void)
{
	static uint32_t div = 0;
	static uint32_t div1 = 0;
	
	div1++;
	if(div1 >= 10)
	{
		div1 = 0;
		//ƽ̨λ�˸���
		AGVImuDataUpdate();
		//ƽ̨ƽ�����
		AGVBalancePdCtrl();
	}
	
	div++;
	if(div >= 100)
	{
		div = 0;
		//���µ�ǰ�ٶ�
		updataSpeed();
		//ִ�е��PID������
		MotorSpeedPidCtrl();
		//ָ����Ч����
	}
}


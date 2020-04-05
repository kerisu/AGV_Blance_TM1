/**
  ******************************************************************************
  * @file    mpu6050.c
  * @author  �Ŷ�
  * @version V1.0.0
  * @date    2019-7-10
  * @brief   �����˶������� MPU6050+HMC5883L ������ٶ� ������ٶ� ����ش� ��������
  ******************************************************************************
  */
	
/*--Include-start-------------------------------------------------------------*/
#include "stm32F4xx.h"

#include "mpu6050.h"
#include "IICcom.h"
#include "filter.h"

//#include "Usartcom.h"
#include "stdio.h"

/*--Include-end---------------------------------------------------------------*/

/* Global data space start----------------------------------------------------*/

// MPU6050 internal address

#define        MPU6050_Addr   		 0xD0        //����������IIC�����еĴӵ�ַ,����ALT  ADDRESS��ַ���Ų�ͬ�޸�

#define        SMPLRT_DIV          0x19        //�����ǲ����ʣ�����ֵ��0x07(125Hz)
#define        CONFIG              0x1A        //��ͨ�˲�Ƶ�ʣ�����ֵ��0x06(5Hz)
#define        GYRO_CONFIG         0x1B        //�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)
#define        ACCEL_CONFIG        0x1C        //���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʣ�����ֵ��0x01(���Լ죬2G��5Hz)
#define        ACCEL_XOUT_H        0x3B        //���ٶȵ�ַ
#define        ACCEL_XOUT_L        0x3C
#define        ACCEL_YOUT_H        0x3D
#define        ACCEL_YOUT_L        0x3E
#define        ACCEL_ZOUT_H        0x3F
#define        ACCEL_ZOUT_L        0x40
#define        TEMP_OUT_H          0x41        //�¶ȵ�ַ
#define        TEMP_OUT_L          0x42

#define        GYRO_XOUT_H         0x43        //�����ǵ�ַ
#define        GYRO_XOUT_L         0x44        
#define        GYRO_YOUT_H         0x45
#define        GYRO_YOUT_L         0x46
#define        GYRO_ZOUT_H         0x47
#define        GYRO_ZOUT_L         0x48

#define        MAG_ADDRESS         0x0c
#define        MAG_XOUT_H          0x04        //������������ַ
#define        MAG_XOUT_L          0x03
#define        MAG_YOUT_H          0x06
#define        MAG_YOUT_L          0x05
#define        MAG_ZOUT_H          0x08
#define        MAG_ZOUT_L          0x07

#define        PWR_MGMT_1          0x6B        //��Դ��������ֵ��0x00(��������)
#define        WHO_AM_I            0x75        //IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��)


LineTypedef 		g_ACCELKB = { .k.x = 1.00,	.b.x = -67.11,			//���ٶ�����
															.k.y = 1.00,	.b.y =  35.88,
															.k.z = 1.00,	.b.z =  891.14,	};

const float 		G_GYRO_G 							= 0.03051756;				//�����ǳ�ʼ������+-2000��ÿ����1 / (65536 / 4000) = 0.03051756*2		
const float 		G_ACCELMAX_1G  				= 4096;							//1G���ٶ�ʱ��ADvalue
const float 		G_GRAVITY_MSS 				= 9.80665f;	

IMUDataTypedef 	g_IMUdata;												//MPU  6050 ����

/* Global data space end------------------------------------------------------*/


void Init_MPU6050(void)
{
/*
   Single_Write(MPU6050_Addr,PWR_M, 0x80);   //
   Single_Write(MPU6050_Addr,SMPL, 0x07);    //
   Single_Write(MPU6050_Addr,DLPF, 0x1E);    //��2000��
   Single_Write(MPU6050_Addr,INT_C, 0x00 );  //
   Single_Write(MPU6050_Addr,PWR_M, 0x00);   //
*/
/*
  Single_Write(MPU6050_Addr,PWR_MGMT_1	, 0x00);        //�������״̬
  Single_Write(MPU6050_Addr,SMPLRT_DIV	, 0x00);
	Single_Write(MPU6050_Addr,CONFIG			, 0x06);
  Single_Write(MPU6050_Addr,GYRO_CONFIG	, 0x18);
  Single_Write(MPU6050_Addr,ACCEL_CONFIG, 0x01);
*/
	
	Single_Write(MPU6050_Addr,PWR_MGMT_1	, 0x00);        //�������״̬
  Single_Write(MPU6050_Addr,SMPLRT_DIV	, 0x00);				// sample rate.  Fsample= 1Khz/(<this value>+1) = 1000Hz
	Single_Write(MPU6050_Addr,CONFIG			, 0x06);				//�ڲ���ͨ�˲�Ƶ�ʣ�98hz 2  5HZ 6
  Single_Write(MPU6050_Addr,GYRO_CONFIG	, 0x10);				//���ٶ�����//1000deg/s
  Single_Write(MPU6050_Addr,ACCEL_CONFIG, 0x10);				//���ٶ�����Accel scale 8g (4096 LSB/g)	
	
	
}

void READ_MPU6050(int16_t * data)
{
	uint8_t low,high;
	
  // read gyroscope data
	
   low=Single_Read(MPU6050_Addr,GYRO_XOUT_L); 
   high=Single_Read(MPU6050_Addr,GYRO_XOUT_H);
   data[0] = (high<<8)|low;

   low=Single_Read(MPU6050_Addr,GYRO_YOUT_L);
   high=Single_Read(MPU6050_Addr,GYRO_YOUT_H);
   data[1]  = (high<<8)|low;
        
   low=Single_Read(MPU6050_Addr,GYRO_ZOUT_L);
   high=Single_Read(MPU6050_Addr,GYRO_ZOUT_H);
   data[2]  = (high<<8)|low;

	
	//temp value
   low=Single_Read(MPU6050_Addr,TEMP_OUT_L); 
   high=Single_Read(MPU6050_Addr,TEMP_OUT_H); 
   data[9]  =(high<<8)|low;

        
		//ACCEL_ZOUT_L
	
		// read acceleration data
   low=Single_Read(MPU6050_Addr,ACCEL_XOUT_L); 
   high=Single_Read(MPU6050_Addr,ACCEL_XOUT_H);
   data[3]  = (high<<8)|low;

   low=Single_Read(MPU6050_Addr,ACCEL_YOUT_L);
   high=Single_Read(MPU6050_Addr,ACCEL_YOUT_H);
   data[4] = (high<<8)|low;
        
   low=Single_Read(MPU6050_Addr,ACCEL_ZOUT_L);
   high=Single_Read(MPU6050_Addr,ACCEL_ZOUT_H);
   data[5] = (high<<8)|low;
}



void bufValueToGA(int16_t * data,IMUDataTypedef * imu)
{
	
	//�¶�ת��
	imu->temp = data[9] / 340.0 + 36.53;															//��ȡ������¶�
	
	//���ٶ�ת��
	imu->gyrcp.x = ((double)data[0]) * G_GYRO_G; 
	imu->gyrcp.y = ((double)data[1]) * G_GYRO_G; 
	imu->gyrcp.z = ((double)data[2]) * G_GYRO_G; 
	
	//���ٶ�ת��
	imu->Accel.x = ((double)data[3] * g_ACCELKB.k.x + g_ACCELKB.b.x) * G_GRAVITY_MSS / G_ACCELMAX_1G;
	imu->Accel.y = ((double)data[4] * g_ACCELKB.k.y + g_ACCELKB.b.y) * G_GRAVITY_MSS / G_ACCELMAX_1G;
	imu->Accel.z = ((double)data[5] * g_ACCELKB.k.z + g_ACCELKB.b.z) * G_GRAVITY_MSS / G_ACCELMAX_1G;
	
}

//���������� �ֶ��������
uint8_t IMUCurveFitting(int16_t * data)
{
	static float Accel_z1[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	static float Accel_z2[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	static float Accel_z3[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
		
	static int16_t num = 0;
		
	float xdata[20] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
		
	float Accel_z_res1[2] = {0,0};
	float Accel_z_res2[2] = {0,0};
	float Accel_z_res3[2] = {0,0};
	
	Accel_z1[num] = data[5]; 
	LSMCurveFitting(xdata,Accel_z1,Accel_z_res1,20);
	float line1 = Accel_z_res1[1] * 10 + Accel_z_res1[0];
	
	Accel_z2[num] = line1;
	LSMCurveFitting(xdata,Accel_z2,Accel_z_res2,20);
	float line2 = Accel_z_res2[1] * 10 + Accel_z_res2[0];
	
//	Accel_z3[num] = line2;
//	LSMCurveFitting(xdata,Accel_z3,Accel_z_res3,20);
//	float line3 = Accel_z_res3[1] * 10 + Accel_z_res3[0];
	
	data[5] = line2;
	
	num++;
	if(num == 20)
	{
		num = 0;
	}
	return 1;
}

IMUDataTypedef getIMUdata(void)
{
		return g_IMUdata;
}
void IMUDataUpdata(void)
{
	int16_t dataBuf[10] = {0,0,0,0,0,0,0,0,0,0,};
	int16_t updataflag = 0;
	
	//��ȡ����������
	READ_MPU6050(dataBuf);
	
	//���������� �ֶ��������
	updataflag = IMUCurveFitting(dataBuf);
	
	if(updataflag)
	{
		//����������ת��
		bufValueToGA(dataBuf,&g_IMUdata);
	}
	
}

//��̬��������ֵ��ȡ��ʾ ��������
void MPU6050debug(void)
{
	uint8_t str [50];
	
	//��ʾ����������
	sprintf((char *)str,"gX = %6.2f,gY = %6.2f,gZ = %6.2f,\r\n",g_IMUdata.gyrcp.x,g_IMUdata.gyrcp.y,g_IMUdata.gyrcp.z);
	//Usart1TxStr(str);
	//��ʾ���ٶ�����
	sprintf((char *)str,"aX = %6.2f,aY = %6.2f,aZ = %6.2f,\r\n",g_IMUdata.Accel.x,g_IMUdata.Accel.y,g_IMUdata.Accel.z);
	//Usart1TxStr(str);
	//��ʾ�¶�����
	sprintf((char *)str,"temp = %6.2f\r\n",g_IMUdata.temp);
	//Usart1TxStr(str);
	
//	sprintf((char *)str,"%d,%d,%d\r\n",g_IMUBuffer[3],g_IMUBuffer[4],g_IMUBuffer[5]);
//	Usart1TxStr(str);
	
	
}




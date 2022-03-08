/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
��ƽ    ̨�������������ܿƼ�K66���İ�
����    д��chiusir
��E-mail  ��chiusir@163.com
������汾��V1.1 ��Ȩ���У���λʹ��������ϵ��Ȩ
�������¡�2020��8��10��
�������Ϣ�ο����е�ַ��
����    վ��http://www.lqist.cn
���Ա����̡�http://longqiu.taobao.com
------------------------------------------------
��dev.env.��IAR/KEIL
��Target �� K60
��Crystal�� 50.000Mhz

��SYS PLL�� 180MHz
________________________________________________________________

QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/

#include <LQ_ICM42605.h>
#include <LQ_GPIO.h>
#include <LQ_GPIO_KEY.h>
#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define SPI_SCK_OUT(n)       PIN_Write(LED0p,n);	//PB0��0
#define SPI_MOSI_OUT(n)      PIN_Write(LED0p,n);	//PB0��0
#define SPI_MISO_IN       PIN_Read(KEY0p);
#define SPI_CS_OUT(n)        PIN_Write(LED0p,n);

#define PI 3.1415926535898
icm_param_t icm_data;
short aacx,aacy,aacz;	        //���ٶȴ�����ԭʼ����
short gyrox,gyroy,gyroz;        //������ԭʼ����
gyro_param_t GyroOffset={0,0,0};               // ������У׼ֵ 
/**
 * @brief ��������Ư��ʼ��
 * ͨ���ɼ�һ���������ֵ�������������ƫ��ֵ��
 * ���� �����Ƕ�ȡ������ - ��Ʈֵ������ȥ�����ƫ������
 */
void gyroOffsetInit(void)     
{
	uint16_t i;
    GyroOffset.Xdata = 0;
    GyroOffset.Ydata = 0;
    GyroOffset.Zdata = 0;
    for (i = 0; i < 200; ++i) 
	{
        ICM_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);    // ��ȡ�����ǽ��ٶ�
        GyroOffset.Xdata += gyrox;
        GyroOffset.Ydata += gyroy;
        GyroOffset.Zdata += gyroz;
        delayms(5);    // ��� 1Khz
    }

    GyroOffset.Xdata /= 200;
    GyroOffset.Ydata /= 200;
    GyroOffset.Zdata /= 200;
}

void icmGetValues(void) 
{
    float alpha = 0.2;
	//ICM_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);
	ICM_Get_Gyroscope(&gyrox,&gyroy,&gyroz);
	ICM_Get_Accelerometer(&aacx,&aacy,&aacz);
    //һ�׵�ͨ�˲�����λg
    icm_data.acc_x = (((float) aacx) * alpha) / 4096 + icm_data.acc_x * (1 - alpha);
    icm_data.acc_y = (((float) aacy) * alpha) / 4096 + icm_data.acc_y * (1 - alpha);
    icm_data.acc_z = (((float) aacz) * alpha) / 4096 + icm_data.acc_z * (1 - alpha);

    //! �����ǽ��ٶȱ���ת��Ϊ�����ƽ��ٶ�: deg/s -> rad/s
    icm_data.gyro_x = ((float) gyrox - GyroOffset.Xdata) * PI / 180 / 16.4f; 
    icm_data.gyro_y = ((float) gyroy - GyroOffset.Ydata) * PI / 180 / 16.4f;
    icm_data.gyro_z = ((float) gyroz - GyroOffset.Zdata) * PI / 180 / 16.4f;
}

#define delta_T     0.008f  // ��������1ms ��Ƶ��1KHZ

float I_ex, I_ey, I_ez;  // ������
quater_param_t Q_info = {1, 0, 0, 0};  // ��Ԫ����ʼ��
euler_param_t eulerAngle;              // ŷ����
float icm_kp= 10.0;    // ���ٶȼƵ��������ʱ������� 
float icm_ki= 0.0001;   // �������������ʵĻ�������
/**
 * @brief �û����˲��㷨������������̬(�����ü��ٶȼ����������ǵĻ������)
 * ���ٶȼƶ���֮��������Ƚ����У��������ݼ��������̬���ţ������Ƕ������������У��������ݿ��ţ�������ʹ�û����������(�ڲ������㷨�Ŵ�̬���)��
 * ���ʹ����̬�����˲����������������ǣ��������ż��ٶȼơ�
 * @tips: n - ��������ϵ�� b - ��������ϵ
 */
void icmAHRSupdate(icm_param_t* icm)            
{
    float halfT = 0.5 * delta_T;    // ��������һ��
    float vx, vy, vz;               // ��ǰ��̬��������������������ϵķ���
    float ex, ey, ez;               // ��ǰ���ټƲ�õ��������ٶ��������ϵķ������õ�ǰ��̬��������������������ϵķ��������
        float norm;
    float q0 = Q_info.q0;  //��Ԫ��
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;
    
    float q0q0 = q0 * q0;  //����ˣ������������
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;

    // ������ֹ״̬Ϊ-g ����������
    if(icm->acc_x * icm->acc_y * icm->acc_z == 0) // �Ӽƴ�����������״̬ʱ(��ʱg = 0)��������̬���㣬��Ϊ�������ĸ���������
        return;

    // �Լ��ٶ����ݽ��й�һ�� �õ���λ���ٶ� (a^b -> ��������ϵ�µļ��ٶ�)

	norm = sqrt(icm->acc_x * icm->acc_x + icm->acc_y * icm->acc_y + icm->acc_z * icm->acc_z); 
    icm->acc_x = icm->acc_x / norm;
    icm->acc_y = icm->acc_y / norm;
    icm->acc_z = icm->acc_z / norm;

    // ��������ϵ���������������ϵķ���
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    // g^b �� a^b ��������ˣ��õ������ǵ�У����������e��ϵ��
    ex = icm->acc_y * vz - icm->acc_z * vy;
    ey = icm->acc_z * vx - icm->acc_x * vz;
    ez = icm->acc_x * vy - icm->acc_y * vx;

    // ����ۼ�
    I_ex += icm_ki * ex;  
    I_ey += icm_ki * ey;
    I_ez += icm_ki * ez;

    // ʹ��PI�������������������(������Ư�����)
    icm->gyro_x = icm->gyro_x + icm_kp* ex + I_ex;
    icm->gyro_y = icm->gyro_y + icm_kp* ey + I_ey;
    icm->gyro_z = icm->gyro_z + icm_kp* ez + I_ez;

    // һ����������������Ԫ��΢�ַ��̣�����halfTΪ�������ڵ�1/2��gx gy gzΪbϵ�����ǽ��ٶȡ�
    q0 = q0 + (-q1 * icm->gyro_x - q2 * icm->gyro_y - q3 * icm->gyro_z) * halfT;
    q1 = q1 + (q0 * icm->gyro_x + q2 * icm->gyro_z - q3 * icm->gyro_y) * halfT;
    q2 = q2 + (q0 * icm->gyro_y - q1 * icm->gyro_z + q3 * icm->gyro_x) * halfT;
    q3 = q3 + (q0 * icm->gyro_z + q1 * icm->gyro_y - q2 * icm->gyro_x) * halfT;

    // ��λ����Ԫ���ڿռ���תʱ�������죬������ת�Ƕȣ������㷨�������Դ�����������任
    norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    Q_info.q0 = q0 / norm;
    Q_info.q1 = q1 / norm;
    Q_info.q2 = q2 / norm;
    Q_info.q3 = q3 / norm;  // ��ȫ�ֱ�����¼��һ�μ������Ԫ��ֵ
}

/*************************************************************************
*  �������ƣ�void SPI_SoftInit(void)
*  ����˵����ģ��SPI�ڳ�ʼ��
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void SPI_SoftInit(void)
{

    PIN_InitConfig(P13_2, PIN_MODE_OUTPUT, 1);
    PIN_InitConfig(P11_3, PIN_MODE_OUTPUT, 1);
    PIN_InitConfig(P15_3, PIN_MODE_OUTPUT, 1);
    PIN_InitConfig(P15_8, PIN_MODE_INPUT, 1);

}
/*************************************************************************
*  �������ƣ�void SPI_SoftReadWriteNbyte(uint8_t *lqbuff, uint16_t len)
*  ����˵����SPI��д���ݼ�����
*  ����˵����uint8_t *buf����ָ��,uint16_t len����
*  �������أ�
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void SPI_SoftReadWriteNbyte(uint8_t *lqbuff, uint16_t len)
{
	uint8_t i;
  SPI_CS_OUT(0);
  SPI_SCK_OUT(1);
  do
  {
    for(i = 0; i < 8; i++)
    {
		delayus(1);
      SPI_MOSI_OUT((*lqbuff) >= 0x80);
      SPI_SCK_OUT(0);
      (*lqbuff) = (*lqbuff)<<1;      
      delayus(1);
      SPI_SCK_OUT(1);
      
      (*lqbuff) |= SPI_MISO_IN;          
    }
    lqbuff++;
  }while(--len);
  SPI_CS_OUT(1);
}
/*************************************************************************
*  �������ƣ�void ICM_Read_Len(uint8_t reg,uint8_t len,uint8_t *buf)
*  ����˵����uint8_t reg��ʼ�Ĵ���,uint8_t *buf����ָ��,uint16_t len����
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void ICM_Read_Len(uint8_t reg,uint8_t len,uint8_t *buf)
{   
  buf[0] = reg | 0x80;
  
  SPI_SoftReadWriteNbyte(buf, len + 1);  
}
/*************************************************************************
*  �������ƣ�uint8_t icm42605_read_reg(uint8_t reg)
*  ����˵������ȡ����
*  ����˵����uint8_t reg�Ĵ�����ַ
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
uint8_t icm42605_read_reg(uint8_t reg)
{
    uint8_t lqbuff[2];
  lqbuff[0] = reg | 0x80;          //�ȷ��ͼĴ���
  
  SPI_SoftReadWriteNbyte(lqbuff, 2);
  
  
  return lqbuff[1];
}
/*************************************************************************
*  �������ƣ�uint8_t icm42605_write_reg(uint8_t reg,uint8_t value)
*  ����˵������Ĵ���д����
*  ����˵����uint8_t reg�Ĵ���,uint8_t value����
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
uint8_t icm42605_write_reg(uint8_t reg,uint8_t value)
{
  uint8_t lqbuff[2];
  
  lqbuff[0] = reg;          //�ȷ��ͼĴ���
  lqbuff[1] = value;        //�ٷ�������
  
  SPI_SoftReadWriteNbyte(lqbuff, 2);//����buff�����ݣ����ɼ��� buff��
  return 0;
}
/*************************************************************************
*  �������ƣ�uint8_t icm42605_read_regs(uint8_t reg,uint8_t *buf,uint16_t len)
*  ����˵����������ȡ����
*  ����˵����uint8_t reg��ʼ�Ĵ���,uint8_t *buf����ָ��,uint16_t len����
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
uint8_t icm42605_read_regs(uint8_t reg,uint8_t *buf,uint16_t len)
{   
  buf[0] = reg | 0x80;
  /* д��Ҫ���ļĴ�����ַ */
  SPI_SoftReadWriteNbyte(buf, len + 1);
  return 0;
}
/*************************************************************************
*  �������ƣ�void ICM_Get_Gyroscope(short *gx,short *gy,short *gz)
*  ����˵����
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void ICM_Get_Gyroscope(short *gx,short *gy,short *gz)
{
  uint8_t buf[7]; 
  ICM_Read_Len(ICM_GYRO_XOUTH_REG,6,buf);
  
  *gx=((uint16_t)buf[1]<<8)|buf[2];  
  *gy=((uint16_t)buf[3]<<8)|buf[4];  
  *gz=((uint16_t)buf[5]<<8)|buf[6];
  
}
/*************************************************************************
*  �������ƣ�void ICM_Get_Accelerometer(short *ax,short *ay,short *az)
*  ����˵����
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void ICM_Get_Accelerometer(short *ax,short *ay,short *az)
{
  uint8_t buf[7];  
  ICM_Read_Len(ICM_ACCEL_XOUTH_REG,6,buf);
  
  *ax=((uint16_t)buf[1]<<8)|buf[2];  
  *ay=((uint16_t)buf[3]<<8)|buf[4];  
  *az=((uint16_t)buf[5]<<8)|buf[6];  
}

/*************************************************************************
*  �������ƣ�void ICM_Get_Raw_data(short *ax,short *ay,short *az,short *gx,short *gy,short *gz)
*  ����˵������ȡ���ٶ�����������
*  ����˵��������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void ICM_Get_Raw_data(short *ax,short *ay,short *az,short *gx,short *gy,short *gz)
{
  uint8_t buf[13];  
  ICM_Read_Len(ICM_ACCEL_XOUTH_REG,12,buf);
  
  *ax=((uint16_t)buf[1]<<8)|buf[2];  
  *ay=((uint16_t)buf[3]<<8)|buf[4];  
  *az=((uint16_t)buf[5]<<8)|buf[6];
  *gx=((uint16_t)buf[7]<<8)|buf[8];  
  *gy=((uint16_t)buf[9]<<8)|buf[10];  
  *gz=((uint16_t)buf[11]<<8)|buf[12];  
}

/*************************************************************************
*  �������ƣ�uint8_t icm42605_init(void)
*  ����˵����ICM42605��ʼ��
*  ����˵������
*  �������أ�0 �ɹ���1ʧ��
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
uint8_t icm42605_init(void)
{   
  uint8_t reg_val;
  
  SPI_SoftInit(); 
  delayms(200);
  
  reg_val = icm42605_read_reg(who_am_i);//who_am_i
 
  icm42605_write_reg(reg_bank_sel,0x00);//Set to bank 0
  icm42605_write_reg(reg_bank_sel,0x00);//Set to bank 0
  icm42605_write_reg(device_config_reg,bit_soft_reset_chip_config);//chip soft reset
  delayms(100);
  
  if(reg_val==0x42)
  {
    icm42605_write_reg(reg_bank_sel,0x01);//Change to bank 1
    icm42605_write_reg(intf_config4,0x02);//4 wire spi mode
    
    icm42605_write_reg(reg_bank_sel,0x00);        
    icm42605_write_reg(fifo_config_reg,0x40);//Stream-to-FIFO Mode
    
    reg_val = icm42605_read_reg(int_source0_reg);      
    icm42605_write_reg(int_source0_reg,0x00);    
    icm42605_write_reg(fifo_config2_reg,0x00);// watermark
    icm42605_write_reg(fifo_config3_reg,0x02);// watermark
    icm42605_write_reg(int_source0_reg, reg_val); 
    icm42605_write_reg(fifo_config1_reg,0x63);// Enable the accel and gyro to the FIFO
    
    icm42605_write_reg(reg_bank_sel,0x00);
    icm42605_write_reg(int_config_reg,0x36);   
    
    icm42605_write_reg(reg_bank_sel, 0x00);
    reg_val = (icm42605_read_reg(int_source0_reg)|bit_int_fifo_ths_int1_en);      
    icm42605_write_reg(int_source0_reg, reg_val);
    
    icm42605_write_reg(reg_bank_sel, 0x00);
    reg_val = ((icm42605_read_reg(accel_config0_reg)&0x1F)|(bit_accel_ui_fs_sel_8g));//8g
    icm42605_write_reg(accel_config0_reg, reg_val);
    
    icm42605_write_reg(reg_bank_sel, 0x00);
    reg_val = ((icm42605_read_reg(accel_config0_reg)&0xF0)|bit_accel_odr_50hz);
    icm42605_write_reg(accel_config0_reg, reg_val); 
    
    icm42605_write_reg(reg_bank_sel, 0x00);
    reg_val = ((icm42605_read_reg(gyro_config0_reg)&0x1F)|(bit_gyro_ui_fs_sel_2000dps));
    icm42605_write_reg(gyro_config0_reg,reg_val);
    
    icm42605_write_reg(reg_bank_sel, 0x00);      
    reg_val = ((icm42605_read_reg(gyro_config0_reg)&0xF0)|bit_gyro_odr_50hz);
    icm42605_write_reg(gyro_config0_reg, reg_val); 
    
    icm42605_write_reg(reg_bank_sel, 0x00);
    reg_val = icm42605_read_reg(pwr_mgmt0_reg)|(bit_accel_mode_ln); // Accel on in LNM
    icm42605_write_reg(pwr_mgmt0_reg, reg_val);  
    delayms(1);
    
    icm42605_write_reg(reg_bank_sel, 0x00);
    reg_val = icm42605_read_reg(pwr_mgmt0_reg)|(bit_gyro_mode_ln); // Gyro on in LNM
    icm42605_write_reg(pwr_mgmt0_reg, reg_val);  
    delayms(1);
    return 0;
  }
  else 
  {
	  while(1)
	  {
	      delayms(500);
	  //LED0_Toggle;
	  }
  }   
  return 1;
}
/*************************************************************************
*  �������ƣ�void icm42605_read_fifo(Sample_data_type_t *data,uint16_t len)
*  ����˵������ȡFIFO
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
uint8_t  fifocount_l, fifocount_h;
uint16_t fifocount;

void icm42605_read_fifo(Sample_data_type_t *data,uint16_t len)
{
	  uint8_t i;
  uint8_t reg_val;
  uint8_t tempbuff[512]={0};
  reg_val = icm42605_read_reg(int_source0_reg);      
  icm42605_write_reg(int_source0_reg,0x00); 

  fifocount_h = icm42605_read_reg(fifo_byte_count_h_res); // Read the FIFO size
  fifocount_l = icm42605_read_reg(fifo_byte_count_l_res);
  fifocount = (fifocount_h<<8)|fifocount_l;
  
  icm42605_read_regs(fifo_data_port,tempbuff,len);
  if(fifocount>=fifo_packet_size) // If we have a complete packet in the FIFO
  {
    for(i=0;i<32;i++)
    {
      if((tempbuff[i*16]&fifo_accel_en)&&(tempbuff[i*16]&fifo_gyro_en))
      {
        data->Sample_accdata[0+i*3] = ((int16_t)((tempbuff[1+i*16] << 8) | tempbuff[2+i*16]))*acc_ssl;
        data->Sample_accdata[1+i*3] = ((int16_t)((tempbuff[3+i*16] << 8) | tempbuff[4+i*16]))*acc_ssl;
        data->Sample_accdata[2+i*3] = ((int16_t)((tempbuff[5+i*16] << 8) | tempbuff[6+i*16]))*acc_ssl;
        data->Sample_gyrdata[0+i*3] = ((int16_t)((tempbuff[7+i*16] << 8) | tempbuff[8+i*16]))/gyr_ssl;
        data->Sample_gyrdata[1+i*3] = ((int16_t)((tempbuff[9+i*16] << 8) | tempbuff[10+i*16]))/gyr_ssl;
        data->Sample_gyrdata[2+i*3] = ((int16_t)((tempbuff[11+i*16]<< 8) | tempbuff[12+i*16]))/gyr_ssl;
      }
    }
  }
  icm42605_write_reg(int_source0_reg, reg_val); 
}
/*************************************************************************
*  �������ƣ�void icm42605_stop(void)
*  ����˵����ֹͣ����
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void icm42605_stop(void)
{
  uint8_t reg_val; 
  icm42605_write_reg(reg_bank_sel, 0x00);
  reg_val = icm42605_read_reg(pwr_mgmt0_reg)&(~bit_accel_mode_ln); // Accel off
  icm42605_write_reg(pwr_mgmt0_reg, reg_val);
  delayms(1);
  
  icm42605_write_reg(reg_bank_sel, 0x00);
  reg_val = icm42605_read_reg(pwr_mgmt0_reg)|(bit_gyro_mode_ln); // Gyro on in LNM
  icm42605_write_reg(pwr_mgmt0_reg, reg_val);   
  delayms(400);
}

/*************************************************************************
*  �������ƣ�void Test_ICM42605FIFO(void)
*  ����˵�������ԣ���ȡ���ٶ�����������
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/
void Test_ICM42605FIFO(void)
{
    //Sample_data_type_t *data;
	float q0 = Q_info.q0;
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;
    //LED_Init();
    //UART_Init(UART4, 115200);

    icm42605_init();
	//gyroOffsetInit();
  while(1)
  {
      delayms(10);
	  
	icmGetValues();
	  icmAHRSupdate(&icm_data);
	  q0 = Q_info.q0;
	  q1 = Q_info.q1;
	  q2 = Q_info.q2;
	  q3 = Q_info.q3;
    // atan2�������������������ԭ��������X�������μнǵĻ���ֵ
    eulerAngle.pitch = asin(2 * q0 * q2 - 2 * q1 * q3) * 180 / PI; 
    eulerAngle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / PI; 
    eulerAngle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / PI;  


  } 
}
/*************************************************************************
*  �������ƣ�void Test_ICM42605FIFO(void)
*  ����˵�������ԣ���ȡ���ٶ�����������
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��8��10��
*  ��    ע��   
*************************************************************************/


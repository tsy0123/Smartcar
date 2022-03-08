/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【平    台】北京龙邱智能科技K66核心板
【编    写】chiusir
【E-mail  】chiusir@163.com
【软件版本】V1.1 版权所有，单位使用请先联系授权
【最后更新】2020年8月10日
【相关信息参考下列地址】
【网    站】http://www.lqist.cn
【淘宝店铺】http://longqiu.taobao.com
------------------------------------------------
【dev.env.】IAR/KEIL
【Target 】 K60
【Crystal】 50.000Mhz

【SYS PLL】 180MHz
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
#define SPI_SCK_OUT(n)       PIN_Write(LED0p,n);	//PB0置0
#define SPI_MOSI_OUT(n)      PIN_Write(LED0p,n);	//PB0置0
#define SPI_MISO_IN       PIN_Read(KEY0p);
#define SPI_CS_OUT(n)        PIN_Write(LED0p,n);

#define PI 3.1415926535898
icm_param_t icm_data;
short aacx,aacy,aacz;	        //加速度传感器原始数据
short gyrox,gyroy,gyroz;        //陀螺仪原始数据
gyro_param_t GyroOffset={0,0,0};               // 陀螺仪校准值 
/**
 * @brief 陀螺仪零漂初始化
 * 通过采集一定数据求均值计算陀螺仪零点偏移值。
 * 后续 陀螺仪读取的数据 - 零飘值，即可去除零点偏移量。
 */
void gyroOffsetInit(void)     
{
	uint16_t i;
    GyroOffset.Xdata = 0;
    GyroOffset.Ydata = 0;
    GyroOffset.Zdata = 0;
    for (i = 0; i < 200; ++i) 
	{
        ICM_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);    // 获取陀螺仪角速度
        GyroOffset.Xdata += gyrox;
        GyroOffset.Ydata += gyroy;
        GyroOffset.Zdata += gyroz;
        delayms(5);    // 最大 1Khz
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
    //一阶低通滤波，单位g
    icm_data.acc_x = (((float) aacx) * alpha) / 4096 + icm_data.acc_x * (1 - alpha);
    icm_data.acc_y = (((float) aacy) * alpha) / 4096 + icm_data.acc_y * (1 - alpha);
    icm_data.acc_z = (((float) aacz) * alpha) / 4096 + icm_data.acc_z * (1 - alpha);

    //! 陀螺仪角速度必须转换为弧度制角速度: deg/s -> rad/s
    icm_data.gyro_x = ((float) gyrox - GyroOffset.Xdata) * PI / 180 / 16.4f; 
    icm_data.gyro_y = ((float) gyroy - GyroOffset.Ydata) * PI / 180 / 16.4f;
    icm_data.gyro_z = ((float) gyroz - GyroOffset.Zdata) * PI / 180 / 16.4f;
}

#define delta_T     0.008f  // 采样周期1ms 即频率1KHZ

float I_ex, I_ey, I_ez;  // 误差积分
quater_param_t Q_info = {1, 0, 0, 0};  // 四元数初始化
euler_param_t eulerAngle;              // 欧拉角
float icm_kp= 10.0;    // 加速度计的收敛速率比例增益 
float icm_ki= 0.0001;   // 陀螺仪收敛速率的积分增益
/**
 * @brief 用互补滤波算法解算陀螺仪姿态(即利用加速度计修正陀螺仪的积分误差)
 * 加速度计对振动之类的噪声比较敏感，长期数据计算出的姿态可信；陀螺仪对振动噪声不敏感，短期数据可信，但长期使用积分误差严重(内部积分算法放大静态误差)。
 * 因此使用姿态互补滤波，短期相信陀螺仪，长期相信加速度计。
 * @tips: n - 导航坐标系； b - 载体坐标系
 */
void icmAHRSupdate(icm_param_t* icm)            
{
    float halfT = 0.5 * delta_T;    // 采样周期一半
    float vx, vy, vz;               // 当前姿态计算得来的重力在三轴上的分量
    float ex, ey, ez;               // 当前加速计测得的重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差
        float norm;
    float q0 = Q_info.q0;  //四元数
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;
    
    float q0q0 = q0 * q0;  //先相乘，方便后续计算
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;

    // 正常静止状态为-g 反作用力。
    if(icm->acc_x * icm->acc_y * icm->acc_z == 0) // 加计处于自由落体状态时(此时g = 0)不进行姿态解算，因为会产生分母无穷大的情况
        return;

    // 对加速度数据进行归一化 得到单位加速度 (a^b -> 载体坐标系下的加速度)

	norm = sqrt(icm->acc_x * icm->acc_x + icm->acc_y * icm->acc_y + icm->acc_z * icm->acc_z); 
    icm->acc_x = icm->acc_x / norm;
    icm->acc_y = icm->acc_y / norm;
    icm->acc_z = icm->acc_z / norm;

    // 载体坐标系下重力在三个轴上的分量
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    // g^b 与 a^b 做向量叉乘，得到陀螺仪的校正补偿向量e的系数
    ex = icm->acc_y * vz - icm->acc_z * vy;
    ey = icm->acc_z * vx - icm->acc_x * vz;
    ez = icm->acc_x * vy - icm->acc_y * vx;

    // 误差累加
    I_ex += icm_ki * ex;  
    I_ey += icm_ki * ey;
    I_ez += icm_ki * ez;

    // 使用PI控制器消除向量积误差(陀螺仪漂移误差)
    icm->gyro_x = icm->gyro_x + icm_kp* ex + I_ex;
    icm->gyro_y = icm->gyro_y + icm_kp* ey + I_ey;
    icm->gyro_z = icm->gyro_z + icm_kp* ez + I_ez;

    // 一阶龙格库塔法求解四元数微分方程，其中halfT为测量周期的1/2，gx gy gz为b系陀螺仪角速度。
    q0 = q0 + (-q1 * icm->gyro_x - q2 * icm->gyro_y - q3 * icm->gyro_z) * halfT;
    q1 = q1 + (q0 * icm->gyro_x + q2 * icm->gyro_z - q3 * icm->gyro_y) * halfT;
    q2 = q2 + (q0 * icm->gyro_y - q1 * icm->gyro_z + q3 * icm->gyro_x) * halfT;
    q3 = q3 + (q0 * icm->gyro_z + q1 * icm->gyro_y - q2 * icm->gyro_x) * halfT;

    // 单位化四元数在空间旋转时不会拉伸，仅有旋转角度，下面算法类似线性代数里的正交变换
    norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    Q_info.q0 = q0 / norm;
    Q_info.q1 = q1 / norm;
    Q_info.q2 = q2 / norm;
    Q_info.q3 = q3 / norm;  // 用全局变量记录上一次计算的四元数值
}

/*************************************************************************
*  函数名称：void SPI_SoftInit(void)
*  功能说明：模拟SPI口初始化
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void SPI_SoftInit(void)
{

    PIN_InitConfig(P13_2, PIN_MODE_OUTPUT, 1);
    PIN_InitConfig(P11_3, PIN_MODE_OUTPUT, 1);
    PIN_InitConfig(P15_3, PIN_MODE_OUTPUT, 1);
    PIN_InitConfig(P15_8, PIN_MODE_INPUT, 1);

}
/*************************************************************************
*  函数名称：void SPI_SoftReadWriteNbyte(uint8_t *lqbuff, uint16_t len)
*  功能说明：SPI读写数据及长度
*  参数说明：uint8_t *buf数据指针,uint16_t len长度
*  函数返回：
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：void ICM_Read_Len(uint8_t reg,uint8_t len,uint8_t *buf)
*  功能说明：uint8_t reg起始寄存器,uint8_t *buf数据指针,uint16_t len长度
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Read_Len(uint8_t reg,uint8_t len,uint8_t *buf)
{   
  buf[0] = reg | 0x80;
  
  SPI_SoftReadWriteNbyte(buf, len + 1);  
}
/*************************************************************************
*  函数名称：uint8_t icm42605_read_reg(uint8_t reg)
*  功能说明：读取数据
*  参数说明：uint8_t reg寄存器地址
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
uint8_t icm42605_read_reg(uint8_t reg)
{
    uint8_t lqbuff[2];
  lqbuff[0] = reg | 0x80;          //先发送寄存器
  
  SPI_SoftReadWriteNbyte(lqbuff, 2);
  
  
  return lqbuff[1];
}
/*************************************************************************
*  函数名称：uint8_t icm42605_write_reg(uint8_t reg,uint8_t value)
*  功能说明：向寄存器写数据
*  参数说明：uint8_t reg寄存器,uint8_t value数据
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
uint8_t icm42605_write_reg(uint8_t reg,uint8_t value)
{
  uint8_t lqbuff[2];
  
  lqbuff[0] = reg;          //先发送寄存器
  lqbuff[1] = value;        //再发送数据
  
  SPI_SoftReadWriteNbyte(lqbuff, 2);//发送buff里数据，并采集到 buff里
  return 0;
}
/*************************************************************************
*  函数名称：uint8_t icm42605_read_regs(uint8_t reg,uint8_t *buf,uint16_t len)
*  功能说明：连续读取数据
*  参数说明：uint8_t reg起始寄存器,uint8_t *buf数据指针,uint16_t len长度
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
uint8_t icm42605_read_regs(uint8_t reg,uint8_t *buf,uint16_t len)
{   
  buf[0] = reg | 0x80;
  /* 写入要读的寄存器地址 */
  SPI_SoftReadWriteNbyte(buf, len + 1);
  return 0;
}
/*************************************************************************
*  函数名称：void ICM_Get_Gyroscope(short *gx,short *gy,short *gz)
*  功能说明：
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：void ICM_Get_Accelerometer(short *ax,short *ay,short *az)
*  功能说明：
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：void ICM_Get_Raw_data(short *ax,short *ay,short *az,short *gx,short *gy,short *gz)
*  功能说明：读取加速度陀螺仪数据
*  参数说明：六轴
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：uint8_t icm42605_init(void)
*  功能说明：ICM42605初始化
*  参数说明：无
*  函数返回：0 成功；1失败
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：void icm42605_read_fifo(Sample_data_type_t *data,uint16_t len)
*  功能说明：读取FIFO
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：void icm42605_stop(void)
*  功能说明：停止工作
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
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
*  函数名称：void Test_ICM42605FIFO(void)
*  功能说明：测试，读取加速度陀螺仪数据
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
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
    // atan2返回输入坐标点与坐标原点连线与X轴正方形夹角的弧度值
    eulerAngle.pitch = asin(2 * q0 * q2 - 2 * q1 * q3) * 180 / PI; 
    eulerAngle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / PI; 
    eulerAngle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / PI;  


  } 
}
/*************************************************************************
*  函数名称：void Test_ICM42605FIFO(void)
*  功能说明：测试，读取加速度陀螺仪数据
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/


#include "ICM42605.h"

#include <LQ_SOFTI2C.h>
#include <LQ_STM.h>
#include <Platform_Types.h>
#include <stdio.h>
#include <LQ_I2C_MPU6050.h>

void delayms_icm42605(uint16 ms)
{
    while(ms--)
    {
        uint16  i = 300;
        while(i--)
        {
            NOP(50);
        }
    }
}
unsigned char ICM4_Get_Raw_data(signed short *ax,signed short *ay,signed short *az,signed short *gx,signed short *gy,signed short *gz)
{
    unsigned char  buf[14],res;
    res=MPU_Read_Len(ICM42605_ADDR,ICM4_ACCEL_XOUTH_REG,12,buf);
    if(res==0)
    {
        *ax=((uint16)buf[0]<<8)|buf[1];
        *ay=((uint16)buf[2]<<8)|buf[3];
        *az=((uint16)buf[4]<<8)|buf[5];
        *gx=((uint16)buf[8]<<8)|buf[9];
        *gy=((uint16)buf[10]<<8)|buf[11];
        *gz=((uint16)buf[12]<<8)|buf[13];
    }
    return res;
}



void ICM42605_Init(void)
{
    signed short  aacx,aacy,aacz;            //加速度传感器原始数据
    signed short  gyrox,gyroy,gyroz;          //陀螺仪原始数据
    uint8_t reg_val;
    IIC_Init();
    int  res;
    delayms(200);
    res = MPU_Read_Byte(ICM42605_ADDR,who_am_i);           //读取MPU6050的ID
    if(res == ICM42605_ID)                                 //器件ID正确
    {
        printf("ICM42605 is OK!\r\n");
    }



    res = 0;
    MPU_Write_Byte(ICM42605_ADDR,ICM_BANK_SEL,0X00);//Set to bank 0
    MPU_Write_Byte(ICM42605_ADDR,ICM_BANK_SEL,0X00);//Set to bank 0
    MPU_Write_Byte(ICM42605_ADDR,device_config_reg,bit_soft_reset_chip_config);//chip soft reset
    delayms(100);  //延时100ms
    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel,0x01);//Change to bank 1
    MPU_Write_Byte(ICM42605_ADDR,intf_config4,0x00);//I2C and I3C

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel,0x00);
    MPU_Write_Byte(ICM42605_ADDR,fifo_config_reg,0x40);//Stream-to-FIFO Mode

    reg_val = MPU_Read_Byte(ICM42605_ADDR,int_source0_reg);
    MPU_Write_Byte(ICM42605_ADDR,int_source0_reg,0x00);
    MPU_Write_Byte(ICM42605_ADDR,fifo_config2_reg,0x00);// watermark
    MPU_Write_Byte(ICM42605_ADDR,fifo_config3_reg,0x02);// watermark
    MPU_Write_Byte(ICM42605_ADDR,int_source0_reg, reg_val);
    MPU_Write_Byte(ICM42605_ADDR,fifo_config1_reg,0x63);// Enable the accel and gyro to the FIFO

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel,0x00);
    MPU_Write_Byte(ICM42605_ADDR,int_config_reg,0x36);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = (MPU_Read_Byte(ICM42605_ADDR,int_source0_reg)|bit_int_fifo_ths_int1_en);
    MPU_Write_Byte(ICM42605_ADDR,int_source0_reg, reg_val);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = ((MPU_Read_Byte(ICM42605_ADDR,accel_config0_reg)&0x1F)|(bit_accel_ui_fs_sel_8g));//8g
    MPU_Write_Byte(ICM42605_ADDR,accel_config0_reg, reg_val);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = ((MPU_Read_Byte(ICM42605_ADDR,accel_config0_reg)&0xF0)|bit_accel_odr_50hz);
    MPU_Write_Byte(ICM42605_ADDR,accel_config0_reg, reg_val);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = ((MPU_Read_Byte(ICM42605_ADDR,gyro_config0_reg)&0x1F)|(bit_gyro_ui_fs_sel_1000dps));
    MPU_Write_Byte(ICM42605_ADDR,gyro_config0_reg,reg_val);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = ((MPU_Read_Byte(ICM42605_ADDR,gyro_config0_reg)&0xF0)|bit_gyro_odr_50hz);
    MPU_Write_Byte(ICM42605_ADDR,gyro_config0_reg, reg_val);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = MPU_Read_Byte(ICM42605_ADDR,pwr_mgmt0_reg)|(bit_accel_mode_ln); // Accel on in LNM
    MPU_Write_Byte(ICM42605_ADDR,pwr_mgmt0_reg, reg_val);
    delayms(1);

    MPU_Write_Byte(ICM42605_ADDR,reg_bank_sel, 0x00);
    reg_val = MPU_Read_Byte(ICM42605_ADDR,pwr_mgmt0_reg)|(bit_gyro_mode_ln); // Gyro on in LNM
    MPU_Write_Byte(ICM42605_ADDR,pwr_mgmt0_reg, reg_val);
    delayms(1);


    while(1)
    {
        ICM4_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);   //得到加速度传感器数据

        delayms(100);
        printf("%d %d %d\r\n", gyrox, gyroy, gyroz);
    }

}

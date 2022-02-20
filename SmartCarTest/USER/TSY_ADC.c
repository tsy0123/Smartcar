/*
 * TSY_ADC.c
 *
 *  Created on: 2022年2月4日
 *      Author: 67552
 */
#include <TSY_ADC.h>
#include <LQ_ADC.h>
#include <LQ_PID.h>
#include <LQ_GPIO.h>
#include <IfxGtm_PinMap.h>
#include <LQ_GTM.h>
//ADC_Read(ADC0);

/*
 * ADC0检测线圈1
 * ADC2检测线圈2
 * ADC5电容电量
 * ADC7电池电量
 * ADC10线圈输入电压
 * ADC11恒功率输出电压
 * ADC12恒功率输出电流
 */
pid_param_t * pid_PowerControl;

float CapPower = 0;

#define CHARGE_PWM          IfxGtm_ATOM0_6_TOUT42_P23_1_OUT
#define CHARGE_FREQUENCY    1000

void ADC_Init(void)
{
    PIN_InitConfig(P23_1, PIN_MODE_OUTPUT, 0);//IN
    PIN_InitConfig(P22_3, PIN_MODE_OUTPUT, 1);//SD

    ATOM_PWM_InitConfig(CHARGE_PWM, 0, CHARGE_FREQUENCY);

    //记得初始化恒功率pid
    //记得初始化I2C
    ADC_InitConfig(ADC0, 80000); //初始化
    ADC_InitConfig(ADC2, 80000);
    ADC_InitConfig(ADC5, 80000);
    ADC_InitConfig(ADC7, 80000);
    ADC_InitConfig(ADC10, 80000);
    ADC_InitConfig(ADC11, 80000);
    ADC_InitConfig(ADC12, 80000);

    pid_PowerControl->target = 50;
}

unsigned short ADC_Read_filter(uint8_t adc, uint8_t count)
{
    unsigned short adcRead = 0;


    uint8 i;
    uint32 sum;

    sum = 0;
    for(i=0; i<count; i++)
    {
        switch(adc)
        {
            case 0:
                adcRead = ADC_Read(ADC0);break;
            case 2:
                adcRead = ADC_Read(ADC2);break;
            case 5:
                adcRead = ADC_Read(ADC5);break;
            case 7:
                adcRead = ADC_Read(ADC7);break;
            case 10:
                adcRead = ADC_Read(ADC10);break;
            case 11:
                adcRead = ADC_Read(ADC11);break;
            case 12:
                adcRead = ADC_Read(ADC12);break;
        }
        sum += adcRead;
    }

    sum = sum/count;
    return (uint16)sum;
}


//恒功率
void wirelessChargeControl(void)
{
    /*思路
     * ADC检测电压电流，转换为功率P=UI
             * PID控制
     */
    float power, I, U;
    I = ADC_Read_filter(12, 10) / 4095 * 3.3 * 50;
    U = ADC_Read_filter(11, 10) / 4095 * 3.3;
    power = U * I;
    PidLocCtrl(pid_PowerControl, power);

    ATOM_PWM_SetDuty(CHARGE_PWM, pid_PowerControl->out, CHARGE_FREQUENCY);

}

void wirelessChargeDetect(void)
{
    float adc_last_pre = 0;
    float adc_last = 0;
    float adc_now = 0;
    adc_now = ADC_Read_filter(5, 10) / 4095 * 3.3;
    if(adc_now > adc_last && adc_last > adc_last_pre)
    {
        //减速
        //检测是否对准
        /*检测思路：
         * 前电感到达阈值、后电感达到阈值，停下
         * 充满后前进
         */

    }
}

void TM1620_Init(void)
{
    TM1620_IIC_SCL_INIT
    TM1620_IIC_SDA_INIT
    TM1620_IIC_SCL_H
    TM1620_IIC_SDA_H

}

void TM1620_DisPrepare(void)
{
    TM1620_IIC_SendByte(0x02);
    TM1620_IIC_SendByte(AddrAutoAdd);
    TM1620_IIC_SendByte(Addr00H);

}
void my_iic_delay()
{
    /* 200MHz 系统时钟下 模拟IIC速度为 400Khz */

    unsigned char  i = 0;
    for(i = 0; i < 30; i++) //修改这里可以调整IIC速率
    {
        __asm("NOP"); /* delay */
    }
}


void LEDChoose(uint8_t num)
{
    //MOS选择
    switch(num)
    {
        case 0:TM1620_IIC_SendByte(0x00);break;
        case 1:TM1620_IIC_SendByte(0x01);break;
        case 2:TM1620_IIC_SendByte(0x03);break;
        case 3:TM1620_IIC_SendByte(0x07);break;
        case 4:TM1620_IIC_SendByte(0x0f);break;
        case 5:TM1620_IIC_SendByte(0x1f);break;
    }
    TM1620_IIC_SendByte(0x00);
}

void LEDControl(uint8_t num)
{
    TM1620_DisPrepare();
     if(num/5==0)
     {
         LEDChoose(num);
         LEDChoose(0);
         LEDChoose(0);
     }
     else if(num/5==1)
     {
         LEDChoose(5);
         LEDChoose(num%5);
         LEDChoose(0);
     }
     else if(num/5==2)
     {
         LEDChoose(5);
         LEDChoose(5);
         LEDChoose(num%5);
     }
     else
     {
         LEDChoose(5);
         LEDChoose(5);
         LEDChoose(5);
     }
     LEDChoose(0);
     LEDChoose(0);
     LEDChoose(0);
     LEDChoose(0);
     TM1620_IIC_SendByte(0x8f);
}

void LEDDisplay(void)
{
    /*初始化8个IO口
     *
     */
    CapPower = ADC_Read_filter(4, 10);
    if(CapPower < 0.8)
    {
        LEDControl(0);
    }
    else if(CapPower < 1.6)
    {
        LEDControl(1);
    }
    else if(CapPower < 2.4)
    {
        LEDControl(2);
    }
    else if(CapPower < 3.2)
    {
        LEDControl(3);
    }
    else if(CapPower < 4.0)
    {
        LEDControl(4);
    }
    else if(CapPower < 4.8)
    {
        LEDControl(5);
    }
    else if(CapPower < 5.6)
    {
        LEDControl(6);
    }
    else if(CapPower < 6.4)
    {
        LEDControl(7);
    }
    else if(CapPower < 7.2)
    {
        LEDControl(8);
    }
    else if(CapPower < 8.0)
    {
        LEDControl(9);
    }
    else if(CapPower < 8.8)
    {
        LEDControl(10);
    }
    else if(CapPower < 9.6)
    {
        LEDControl(11);
    }
    else if(CapPower < 10.4)
    {
        LEDControl(12);
    }
    else if(CapPower < 11.2)
    {
        LEDControl(13);
    }
    else if(CapPower < 12.0)
    {
        LEDControl(14);
    }
    else if(CapPower < 12.1)
    {
        LEDControl(15);
    }

}

/*************************************************************************
*  函数名称：void TM1620_IIC_Start(void)
*  功能说明：模拟IIC起始信号
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  应用举例：IIC_Start();
*************************************************************************/
void TM1620_IIC_Start(void)
{
    TM1620_SDA_OUT;   //sda线输出
    TM1620_IIC_SDA_H;
    TM1620_IIC_SCL_H;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SDA_L; //START:when CLK is high,DATA change form high to low
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_L; //钳住I2C总线，准备发送或接收数据
}


/*************************************************************************
*  函数名称：void TM1620_IIC_Stop(void)
*  功能说明：模拟IIC停止信号
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  应用举例：IIC_Stop();
*************************************************************************/
void TM1620_IIC_Stop(void)
{
    TM1620_SDA_OUT; //sda线输出
    TM1620_IIC_SCL_L;
    TM1620_IIC_SDA_L; //STOP:when CLK is high DATA change form low to high
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_H;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SDA_H; //发送I2C总线结束信号
    my_iic_delay();
}


/*************************************************************************
*  函数名称：unsigned char TM1620_IIC_WaitAck(void)
*  功能说明：模拟IIC等待应答信号
*  参数说明：无
*  函数返回：1，接收应答失败    0，接收应答成功
*  修改时间：2020年3月10日
*  应用举例：内部调用 有效应答：从机第9个 SCL=0 时 SDA 被从机拉低,并且 SCL = 1时 SDA依然为低
*************************************************************************/
unsigned char TM1620_IIC_WaitAck(void)
{
    unsigned char  ucErrTime=0;
    TM1620_SDA_IN; //SDA设置为输入  （从机给一个低电平做为应答）
    TM1620_IIC_SDA_H;my_iic_delay();
    TM1620_IIC_SCL_H;my_iic_delay();
    while(TM1620_IIC_SDA_READ)
    {
        ucErrTime++;
        if(ucErrTime>100)
        {
            TM1620_IIC_Stop();
            return 1;
        }
    }
    TM1620_IIC_SCL_L; //时钟输出0
    return 0;
}

/*************************************************************************
*  函数名称：void TM1620_IIC_Ack(void)
*  功能说明：模拟IIC产生ACK应答
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  应用举例：内部调用 主机接收完一个字节数据后，主机产生的ACK通知从机一个字节数据已正确接收
*************************************************************************/
void TM1620_IIC_Ack(void)
{
    TM1620_IIC_SCL_L;
    TM1620_SDA_OUT;
    TM1620_IIC_SDA_L;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_H;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_L;
}


/*************************************************************************
*  函数名称：void TM1620_IIC_NAck(void)
*  功能说明：模拟IIC不产生ACK应答
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  应用举例：内部调用 主机接收完最后一个字节数据后，主机产生的NACK通知从机发送结束，释放SDA,以便主机产生停止信号
*************************************************************************/
void TM1620_IIC_NAck(void)
{
    TM1620_IIC_SCL_L;
    TM1620_SDA_OUT;
    TM1620_IIC_SDA_H;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_H;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_L;
}


/*************************************************************************
*  函数名称：void TM1620_IIC_SendByte(unsigned char data_t)
*  功能说明：模拟IIC发送一个字节
*  参数说明：data   :  发送的字节
*  函数返回：无
*  修改时间：2020年3月10日
*  应用举例：IIC_SendByte(0x12);
*************************************************************************/
void TM1620_IIC_SendByte(unsigned char data_t)
{
    unsigned char  t;
    TM1620_SDA_OUT;
    TM1620_IIC_SCL_L; //拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {
//        IIC_SDA_READ = data_t&0x80;
        if(data_t&0x80)
        {
            TM1620_IIC_SDA_H;
        }
        else
        {
            TM1620_IIC_SDA_L;
        }

        TM1620_IIC_SCL_H;;
        my_iic_delay();
        data_t<<=1;
        my_iic_delay();
        my_iic_delay();
        TM1620_IIC_SCL_L;
        my_iic_delay();
    }
    my_iic_delay();
}


/*************************************************************************
*  函数名称：unsigned char TM1620_IIC_ReadByte(unsigned char ack)
*  功能说明：模拟IIC读取一个字节
*  参数说明：ack=1 时，主机数据还没接收完 ack=0 时主机数据已全部接收完成
*  函数返回：接收到的字节
*  修改时间：2020年3月10日
*  应用举例：IC_ReadByte(0x12);
*************************************************************************/
unsigned char TM1620_IIC_ReadByte(unsigned char ack)
{
    unsigned char  i,receive=0;
    TM1620_SDA_IN; //SDA设置为输入模式 等待接收从机返回数据
    for(i=0;i<8;i++ )
    {
        TM1620_IIC_SCL_L;
        my_iic_delay();
        my_iic_delay();
        my_iic_delay();
        TM1620_IIC_SCL_H;
        receive<<=1;
        if(TM1620_IIC_SDA_READ)receive++; //从机发送的电平
        my_iic_delay();
    }
    if(ack)
        TM1620_IIC_Ack(); //发送ACK
    else
        TM1620_IIC_NAck(); //发送nACK
    return receive;
}



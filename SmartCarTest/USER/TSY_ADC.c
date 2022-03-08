/*
 * TSY_ADC.c
 *
 *  Created on: 2022��2��4��
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
 * ADC0�����Ȧ1
 * ADC2�����Ȧ2
 * ADC5���ݵ���
 * ADC7��ص���
 * ADC10��Ȧ�����ѹ
 * ADC11�㹦�������ѹ
 * ADC12�㹦���������
 */
pid_param_t * pid_PowerControl;

float CapPower = 0;

#define CHARGE_PWM          IfxGtm_ATOM0_6_TOUT42_P23_1_OUT
#define CHARGE_FREQUENCY    1000

void ADC_Init_TSY(void)
{
    PIN_InitConfig(P23_1, PIN_MODE_OUTPUT, 0);//IN
    PIN_InitConfig(P22_3, PIN_MODE_OUTPUT, 1);//SD

    //ATOM_PWM_InitConfig(CHARGE_PWM, 0, CHARGE_FREQUENCY);
    TM1620_Init();
    LEDControl(0);

    //PidSet(pid_PowerControl, 0.0, 0.0, 0.0, 90000.0);
    /*ADC_InitConfig(SIGNAL_DETECT_1, 80000); //��ʼ��
    ADC_InitConfig(SIGNAL_DETECT_2, 80000);
    ADC_InitConfig(CAPACITOR_VOLTAGE, 80000);
    ADC_InitConfig(BATTERY_VOTAGE, 80000);
    ADC_InitConfig(VOLTAGE_IN, 80000);
    ADC_InitConfig(CONSTANT_VOLTAGE_OUT, 80000);
    ADC_InitConfig(CONSTANT_CURRENT_OUT, 80000);*/

    //pid_PowerControl->target = 50;
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
            case ADC_SIGNAL_1:
                adcRead = ADC_Read(ADC0);break;
            case ADC_SIGNAL_2:
                adcRead = ADC_Read(ADC2);break;
            case ADC_CAPACITOR_VOLTAGE:
                adcRead = ADC_Read(ADC5);break;
            case ADC_BATTERY_VOTAGE:
                adcRead = ADC_Read(ADC7);break;
            case ADC_VOLTAGE_IN:
                adcRead = ADC_Read(ADC10);break;
            case ADC_CONSTANT_VOLTAGE_OUT:
                adcRead = ADC_Read(ADC11);break;
            case ADC_CONSTANT_CURRENT_OUT:
                adcRead = ADC_Read(ADC12);break;
        }
        sum += adcRead;
    }

    sum = sum/count;
    return (uint16)sum;
}


//�㹦��
void wirelessChargeControl(void)
{
    /*˼·
     * ADC����ѹ������ת��Ϊ����P=UI
             * PID����
     */
    float power, I, U;
    I = ADC_Read_filter(ADC_CONSTANT_CURRENT_OUT, 10) / 4095 * 3.3 * 50;
    U = ADC_Read_filter(ADC_CONSTANT_VOLTAGE_OUT, 10) / 4095 * 3.3;
    power = U * I;
    PidLocCtrl(pid_PowerControl, power);

    ATOM_PWM_SetDuty(CHARGE_PWM, pid_PowerControl->out, CHARGE_FREQUENCY);

}

void wirelessChargeDetect(void)
{
    float adc_last_pre = 0;
    float adc_last = 0;
    float adc_now = 0;
    adc_now = ADC_Read_filter(ADC_CAPACITOR_VOLTAGE, 10) / 4095 * 3.3;
    if(adc_now > adc_last && adc_last > adc_last_pre)
    {
        //����
        //����Ƿ��׼
        /*���˼·��
         * ǰ��е�����ֵ�����дﵽ��ֵ��ͣ��
         * ������ǰ��
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
    /* 200MHz ϵͳʱ���� ģ��IIC�ٶ�Ϊ 400Khz */

    unsigned char  i = 0;
    for(i = 0; i < 30; i++) //�޸�������Ե���IIC����
    {
        __asm("NOP"); /* delay */
    }
}


void LEDChoose(uint8_t num)
{
    //MOSѡ��
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
    switch(num/3)
    {
        case 0:
            LEDChoose(num);
            LEDChoose(0);
            LEDChoose(0);
            LEDChoose(0);
            LEDChoose(0);
            break;
        case 1:
            LEDChoose(3);
            LEDChoose(num%3);
            LEDChoose(0);
            LEDChoose(0);
            LEDChoose(0);
            break;
        case 2:
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(num%3);
            LEDChoose(0);
            LEDChoose(0);
            break;
        case 3:
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(num%3);
            LEDChoose(0);
            break;
        case 4:
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(num%3);
            break;
        default:
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(3);
            LEDChoose(3);
            break;
     }
     LEDChoose(0);
     LEDChoose(0);
     TM1620_IIC_SendByte(0x8f);
}

void LEDDisplay(void)
{
    /*��ʼ��2��IO��
     *
     */
    CapPower = ADC_Read_filter(ADC_CAPACITOR_VOLTAGE, 10);
    LEDControl(CapPower/0.8);
}

/*************************************************************************
*  �������ƣ�void TM1620_IIC_Start(void)
*  ����˵����ģ��IIC��ʼ�ź�
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ�����IIC_Start();
*************************************************************************/
void TM1620_IIC_Start(void)
{
    TM1620_SDA_OUT;   //sda�����
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
    TM1620_IIC_SCL_L; //ǯסI2C���ߣ�׼�����ͻ��������
}


/*************************************************************************
*  �������ƣ�void TM1620_IIC_Stop(void)
*  ����˵����ģ��IICֹͣ�ź�
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ�����IIC_Stop();
*************************************************************************/
void TM1620_IIC_Stop(void)
{
    TM1620_SDA_OUT; //sda�����
    TM1620_IIC_SCL_L;
    TM1620_IIC_SDA_L; //STOP:when CLK is high DATA change form low to high
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SCL_H;
    my_iic_delay();
    my_iic_delay();
    my_iic_delay();
    TM1620_IIC_SDA_H; //����I2C���߽����ź�
    my_iic_delay();
}


/*************************************************************************
*  �������ƣ�unsigned char TM1620_IIC_WaitAck(void)
*  ����˵����ģ��IIC�ȴ�Ӧ���ź�
*  ����˵������
*  �������أ�1������Ӧ��ʧ��    0������Ӧ��ɹ�
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ������ڲ����� ��ЧӦ�𣺴ӻ���9�� SCL=0 ʱ SDA ���ӻ�����,���� SCL = 1ʱ SDA��ȻΪ��
*************************************************************************/
unsigned char TM1620_IIC_WaitAck(void)
{
    unsigned char  ucErrTime=0;
    TM1620_SDA_IN; //SDA����Ϊ����  ���ӻ���һ���͵�ƽ��ΪӦ��
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
    TM1620_IIC_SCL_L; //ʱ�����0
    return 0;
}

/*************************************************************************
*  �������ƣ�void TM1620_IIC_Ack(void)
*  ����˵����ģ��IIC����ACKӦ��
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ������ڲ����� ����������һ���ֽ����ݺ�����������ACK֪ͨ�ӻ�һ���ֽ���������ȷ����
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
*  �������ƣ�void TM1620_IIC_NAck(void)
*  ����˵����ģ��IIC������ACKӦ��
*  ����˵������
*  �������أ���
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ������ڲ����� �������������һ���ֽ����ݺ�����������NACK֪ͨ�ӻ����ͽ������ͷ�SDA,�Ա���������ֹͣ�ź�
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
*  �������ƣ�void TM1620_IIC_SendByte(unsigned char data_t)
*  ����˵����ģ��IIC����һ���ֽ�
*  ����˵����data   :  ���͵��ֽ�
*  �������أ���
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ�����IIC_SendByte(0x12);
*************************************************************************/
void TM1620_IIC_SendByte(unsigned char data_t)
{
    unsigned char  t;
    TM1620_SDA_OUT;
    TM1620_IIC_SCL_L; //����ʱ�ӿ�ʼ���ݴ���
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
*  �������ƣ�unsigned char TM1620_IIC_ReadByte(unsigned char ack)
*  ����˵����ģ��IIC��ȡһ���ֽ�
*  ����˵����ack=1 ʱ���������ݻ�û������ ack=0 ʱ����������ȫ���������
*  �������أ����յ����ֽ�
*  �޸�ʱ�䣺2020��3��10��
*  Ӧ�þ�����IC_ReadByte(0x12);
*************************************************************************/
unsigned char TM1620_IIC_ReadByte(unsigned char ack)
{
    unsigned char  i,receive=0;
    TM1620_SDA_IN; //SDA����Ϊ����ģʽ �ȴ����մӻ���������
    for(i=0;i<8;i++ )
    {
        TM1620_IIC_SCL_L;
        my_iic_delay();
        my_iic_delay();
        my_iic_delay();
        TM1620_IIC_SCL_H;
        receive<<=1;
        if(TM1620_IIC_SDA_READ)receive++; //�ӻ����͵ĵ�ƽ
        my_iic_delay();
    }
    if(ack)
        TM1620_IIC_Ack(); //����ACK
    else
        TM1620_IIC_NAck(); //����nACK
    return receive;
}



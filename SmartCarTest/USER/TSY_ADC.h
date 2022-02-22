
#define TM1620_IIC_SCL_PIN   P13_1   /*!< P13_1  ��Ϊ SCL */
#define TM1620_IIC_SDA_PIN   P13_2   /*!< P13_2  ��Ϊ SDA */

#define TM1620_SDA_OUT        PIN_Dir(TM1620_IIC_SDA_PIN, 1);
#define TM1620_SDA_IN         PIN_Dir(TM1620_IIC_SDA_PIN, 0);

#define TM1620_IIC_SCL_INIT   PIN_InitConfig(TM1620_IIC_SCL_PIN, PIN_MODE_OUTPUT, 1);
#define TM1620_IIC_SDA_INIT   PIN_InitConfig(TM1620_IIC_SDA_PIN, PIN_MODE_OUTPUT, 1);

#define TM1620_IIC_SCL_H      PIN_Write(TM1620_IIC_SCL_PIN, 1);
#define TM1620_IIC_SCL_L      PIN_Write(TM1620_IIC_SCL_PIN, 0);

#define TM1620_IIC_SDA_H      PIN_Write(TM1620_IIC_SDA_PIN, 1);
#define TM1620_IIC_SDA_L      PIN_Write(TM1620_IIC_SDA_PIN, 0);

#define TM1620_IIC_SDA_READ   PIN_Read(TM1620_IIC_SDA_PIN)

#define AddrAutoAdd     0x40//д��ʾ���Զ��ۼӵ�ַ

#define Addr00H             0xC0//��ַ00H

#define ModeDisTM1620  0x02

#define SIGNAL_DETECT_1         0x0000
#define SIGNAL_DETECT_2         0x0202
#define CAPACITOR_VOLTAGE       0x0505
#define BATTERY_VOTAGE          0x0707
#define VOLTAGE_IN              0x0A09
#define CONSTANT_VOLTAGE_OUT    0x0B0A
#define CONSTANT_CURRENT_OUT    0x0C0B

enum TSY_Charge_ADC_Channel
{
    ADC_SIGNAL_1 = 0,
    ADC_SIGNAL_2,
    ADC_CAPACITOR_VOLTAGE,
    ADC_BATTERY_VOTAGE,
    ADC_VOLTAGE_IN,
    ADC_CONSTANT_VOLTAGE_OUT,
    ADC_CONSTANT_CURRENT_OUT,
}TSY_Charge_ADC_t;
void TM1620_IIC_Start(void);               //����IIC��ʼ�ź�
void TM1620_IIC_Stop(void);                //����IICֹͣ�ź�
void TM1620_IIC_Ack(void);                 //IIC����ACK�ź�
void TM1620_IIC_NAck(void);                //IIC������ACK�ź�
unsigned char TM1620_IIC_WaitAck(void);            //IIC�ȴ�ACK�ź�
void TM1620_IIC_SendByte(unsigned char data_t);    //IIC����һ���ֽ�
unsigned char TM1620_IIC_ReadByte(unsigned char ack);      //IIC��ȡһ���ֽ�


void TM1620_IIC_Init(void);


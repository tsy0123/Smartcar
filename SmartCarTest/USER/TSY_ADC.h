/* ע�� IIC���߹涨��IIC����ʱ SCL��SDA��Ϊ�ߵ�ƽ ����ⲿ������һ������������ */
/* ģ�� IIC��Ҫע�⣬IIC��ַ����һλ ����MPU6050 ģ����ǵ�ַ 0xD0 */
/* �뻻�ñ��IO ֱ���޸ĺ궨�� SOFT_IIC_SCL_PIN �� SOFT_IIC_SDA_PIN ���� */
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

void TM1620_IIC_Start(void);               //����IIC��ʼ�ź�
void TM1620_IIC_Stop(void);                //����IICֹͣ�ź�
void TM1620_IIC_Ack(void);                 //IIC����ACK�ź�
void TM1620_IIC_NAck(void);                //IIC������ACK�ź�
unsigned char TM1620_IIC_WaitAck(void);            //IIC�ȴ�ACK�ź�
void TM1620_IIC_SendByte(unsigned char data_t);    //IIC����һ���ֽ�
unsigned char TM1620_IIC_ReadByte(unsigned char ack);      //IIC��ȡһ���ֽ�


void TM1620_IIC_Init(void);


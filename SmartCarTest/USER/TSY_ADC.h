/* 注意 IIC总线规定，IIC空闲时 SCL和SDA都为高电平 最好外部上拉（一定不能下拉） */
/* 模拟 IIC需要注意，IIC地址左移一位 例如MPU6050 模拟就是地址 0xD0 */
/* 想换用别的IO 直接修改宏定义 SOFT_IIC_SCL_PIN 、 SOFT_IIC_SDA_PIN 即可 */
#define TM1620_IIC_SCL_PIN   P13_1   /*!< P13_1  作为 SCL */
#define TM1620_IIC_SDA_PIN   P13_2   /*!< P13_2  作为 SDA */

#define TM1620_SDA_OUT        PIN_Dir(TM1620_IIC_SDA_PIN, 1);
#define TM1620_SDA_IN         PIN_Dir(TM1620_IIC_SDA_PIN, 0);

#define TM1620_IIC_SCL_INIT   PIN_InitConfig(TM1620_IIC_SCL_PIN, PIN_MODE_OUTPUT, 1);
#define TM1620_IIC_SDA_INIT   PIN_InitConfig(TM1620_IIC_SDA_PIN, PIN_MODE_OUTPUT, 1);

#define TM1620_IIC_SCL_H      PIN_Write(TM1620_IIC_SCL_PIN, 1);
#define TM1620_IIC_SCL_L      PIN_Write(TM1620_IIC_SCL_PIN, 0);

#define TM1620_IIC_SDA_H      PIN_Write(TM1620_IIC_SDA_PIN, 1);
#define TM1620_IIC_SDA_L      PIN_Write(TM1620_IIC_SDA_PIN, 0);

#define TM1620_IIC_SDA_READ   PIN_Read(TM1620_IIC_SDA_PIN)

#define AddrAutoAdd     0x40//写显示，自动累加地址

#define Addr00H             0xC0//地址00H

#define ModeDisTM1620  0x02

void TM1620_IIC_Start(void);               //发送IIC开始信号
void TM1620_IIC_Stop(void);                //发送IIC停止信号
void TM1620_IIC_Ack(void);                 //IIC发送ACK信号
void TM1620_IIC_NAck(void);                //IIC不发送ACK信号
unsigned char TM1620_IIC_WaitAck(void);            //IIC等待ACK信号
void TM1620_IIC_SendByte(unsigned char data_t);    //IIC发送一个字节
unsigned char TM1620_IIC_ReadByte(unsigned char ack);      //IIC读取一个字节


void TM1620_IIC_Init(void);


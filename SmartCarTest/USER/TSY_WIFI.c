#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>
#include <LQ_UART.h> 
#include <LQ_PID.h>
#include <Drive.h>
#include <TSY_WIFI.h>
void TSY_Bluetooth(void)
{
    UART_InitConfig(UART0_RX_P14_1,UART0_TX_P14_0, 115200);
    UART_PutStr(UART0,"UART0 LongQiu \r\n");                //发送字符串到上位机
}
extern IfxAsclin_Asc g_UartConfig[4];
/*************************************************************************
*  函数名称：void UART0_RX_IRQHandler(void)
*  功能说明：UART0_RX_IRQHandler中断服务函数
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月30日
*  备    注：
*************************************************************************/
uint8_t USART_RX_BUF[100];     //接收缓冲,最大8个字节.
uint16_t USART_RX_STA=0;       //接收状态标记
uint8_t aRxBuffer[1];
uint8_t waySwitch = 0;
uint8_t manControl = 0;
uint8_t startWIFI = 0;
uint8_t stopWIFI = 0;
extern bool WIFI_show_Pic;
extern bool WIFI_show_Chart;
extern bool WIFIInitIsOk;
void UART0_RX_IRQHandler(void)
{
    Ifx_SizeT count = 1;
    IfxAsclin_Asc_isrReceive(&g_UartConfig[0]);
    IfxAsclin_Asc_read(&g_UartConfig[0], aRxBuffer, &count, TIME_INFINITE);

    float p = 0, i = 0, d = 0, im = 0;
    /* 用户代码 */
    //UART_PutChar(UART0, aRxBuffer[0]);
    if(WIFIInitIsOk)
        if((USART_RX_STA&0x8000)==0)//接收未完成
        {

            if(aRxBuffer[0]=='E')
            {
                if(USART_RX_STA!=14) USART_RX_STA=0;
                else USART_RX_STA=0x8000;
            }
            else if((aRxBuffer[0]>='0'&&aRxBuffer[0]<='9')||(aRxBuffer[0]>='a'&&aRxBuffer[0]<='z')||(aRxBuffer[0]>='A'&&aRxBuffer[0]<='Z'))
            {
                USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0];
                USART_RX_STA++;
                if(USART_RX_STA>14) USART_RX_STA=0;
            }
            else
            {
                //LED_Ctrl(LED0,ON);
                USART_RX_BUF[USART_RX_STA&0X3FFF]='0';
                USART_RX_STA++;
                if(USART_RX_STA>14) USART_RX_STA=0;
            }

        }
    if(USART_RX_STA==0x8000)
    {

        if(USART_RX_BUF[13]=='l'||USART_RX_BUF[13]=='r'||USART_RX_BUF[13]=='d')
        {
            //LED_Ctrl(LED0,OFF);
            p = (USART_RX_BUF[0]-0x30)*10 + (USART_RX_BUF[1]-0x30)*1 + (USART_RX_BUF[2]-0x30)*0.1 + (USART_RX_BUF[3]-0x30)*0.01;
            i = (USART_RX_BUF[4]-0x30)*10 + (USART_RX_BUF[5]-0x30)*1 + (USART_RX_BUF[6]-0x30)*0.1 + (USART_RX_BUF[7]-0x30)*0.01;
            d = (USART_RX_BUF[8]-0x30)*10 + (USART_RX_BUF[9]-0x30)*1 + (USART_RX_BUF[10]-0x30)*0.1 + (USART_RX_BUF[11]-0x30)*0.01;
            im = (USART_RX_BUF[12] - 0x30) * 1000;
            switch(USART_RX_BUF[13])
            {
                case 'd':PidSet(&pid_Drive, p, i, d, im);break;
                case 'l':PidSet(&pid_Left, p, i, d, im);break;
                case 'r':PidSet(&pid_Right, p, i, d, im);break;
            }
        }
        else if(USART_RX_BUF[13]=='L'||USART_RX_BUF[13]=='R'||USART_RX_BUF[13]=='W'||USART_RX_BUF[13]=='s'||USART_RX_BUF[13]=='t')
        {
            //LED_Ctrl(LED0,OFF);
            switch(USART_RX_BUF[13])
            {
                case 'L': waySwitch = 1;break;
                case 'R': waySwitch = 2;break;
                case 'W': waySwitch = 3;break;
                case 's': waySwitch = 4;break;
                case 't': waySwitch = 0;break;
            }
        }
        else if(USART_RX_BUF[13]=='M'||USART_RX_BUF[13]=='A')
        {
            //LED_Ctrl(LED0,OFF);
            switch(USART_RX_BUF[13])
            {
                case 'M': manControl = 1;break;
                case 'A': manControl = 0;break;
            }
        }
        else if(USART_RX_BUF[13]=='S')
        {
            //LED_Ctrl(LED0,OFF);
            roadSpeed = (USART_RX_BUF[0]-0x30)*1000 + (USART_RX_BUF[1]-0x30)*100 + (USART_RX_BUF[2]-0x30)*10 + (USART_RX_BUF[3]-0x30)*1;
        }
        else if(USART_RX_BUF[13]=='C')
        {
            //LED_Ctrl(LED0,OFF);
            if(USART_RX_BUF[0]=='1')
            {
                WIFI_show_Chart = false;
                WIFI_show_Pic=true;
            }
            else if(USART_RX_BUF[1]=='1')
            {
                WIFI_show_Pic = false;
                WIFI_show_Chart = true;
            }
            else
            {
                WIFI_show_Pic = false;
                WIFI_show_Chart = false;
            }
        }
        USART_RX_STA = 0;
        //UART_PutStr(UART0, "OK!\r\n");
    }



}

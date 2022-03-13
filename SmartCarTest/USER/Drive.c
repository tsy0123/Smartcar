#include <LQ_PID.h>
#include <LQ_GPT12_ENC.h>
#include <Drive.h>
#include <TSY_WIFI.h>
pid_param_t pid_Drive;
pid_param_t pid_Left;
pid_param_t pid_Right;
int roadSpeed = 150;
int turnSpeed;
int time = 0;
float CenterERR = 0;
float CenterERR_Last = 0;
extern bool WIFI_show_Chart;
//直线速度 201.06192982974676726160917652989*编码器/1024*100*30/68/1000 (m/s)
//0.00866247974335420929723155344654*编码器
//设定编码器值150时，速度为1.3BAT_ADCm/s
//173:  1.5m/s
//208:  1.8m/s
//231:  2.0m/s

void TSY_MortorCtrl(sint32 motor1, sint32 motor2)
{
    if(motor1 >= 0)
    {
        //ATOM_PWM_SetDuty(MOTOR1_P, motor1, MOTOR_FREQUENCY);
        PIN_Write(MOTOR1_P,1);
        ATOM_PWM_SetDuty(MOTOR1_N, motor1, MOTOR_FREQUENCY);
    }
    else
    {
        PIN_Write(MOTOR1_P,0);
        ATOM_PWM_SetDuty(MOTOR1_N, -motor1, MOTOR_FREQUENCY);
    }
    if(motor2 >= 0)
    {
        PIN_Write(MOTOR2_P,1);
        ATOM_PWM_SetDuty(MOTOR2_N, motor2, MOTOR_FREQUENCY);
    }
    else
    {
        PIN_Write(MOTOR2_P,0);
        ATOM_PWM_SetDuty(MOTOR2_N, -motor2, MOTOR_FREQUENCY);
    }

}
void TSY_Drive(pid_param_t *pidDrive, pid_param_t *pidLeft, pid_param_t *pidRight)
{
    short ENCLeft = ENC_GetCounter(ENC2_InPut_P33_7);
    short ENCRight = -ENC_GetCounter(ENC6_InPut_P20_3);
    short MotorLeft = 0;
    short MotorRight = 0;
    float speedSet = (float)roadSpeed;
    /*if((camERR.cam_finalCenterERR[0]>-4 && camERR.cam_finalCenterERR[0] < 4)\
            && (camERR.cam_finalCenterERR[1]>-4 && camERR.cam_finalCenterERR[1] < 4)\
            && (camERR.cam_finalCenterERR[2]>-4 && camERR.cam_finalCenterERR[2] < 4))
        speedSet = roadSpeed * 1.2;
    else
        speedSet = (float)roadSpeed;*/
    if(WIFI_show_Chart == true)
        time++;
    if(time == 3)
    {
        printf("C%d %d \r\n",ENCLeft,ENCRight);
        time = 0;
    }
    if(!manControl)
    {
        CenterERR = camERR.cam_finalCenterERR[0]*0.5 + CenterERR_Last*0.5;
        CenterERR_Last = CenterERR;
        PidLocCtrl(pidDrive, CenterERR);
        pidSetTarget(pidLeft, speedSet - pidDrive->out);
        pidSetTarget(pidRight, speedSet + pidDrive->out);
        PidLocCtrl(pidLeft, ENCLeft);
        PidLocCtrl(pidRight, ENCRight);

    }
    else
    {
        switch(waySwitch)
        {
            case 3:
                pidSetTarget(pidLeft, (float)roadSpeed);
                pidSetTarget(pidRight, (float)roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                break;
            case 2:
                pidSetTarget(pidLeft, (float)roadSpeed);
                pidSetTarget(pidRight, -(float)roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                break;
            case 1:
                pidSetTarget(pidLeft, -(float)roadSpeed);
                pidSetTarget(pidRight, (float)roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                break;
            case 4:
                pidSetTarget(pidLeft, -(float)roadSpeed);
                pidSetTarget(pidRight, (float)-roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                break;
            default:
                pidSetTarget(pidLeft, 0);
                pidSetTarget(pidRight, 0);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                break;
        }
    }
    MotorLeft = pidLeft->out;
    MotorRight = pidRight->out;
    if(MotorLeft>10000) MotorLeft = 10000;
    if(MotorLeft<-10000) MotorLeft = -10000;
    if(MotorRight>10000) MotorRight = 10000;
    if(MotorRight<-10000) MotorRight = -10000;
    TSY_MortorCtrl(MotorLeft,MotorRight);
}

void MotorInit_TSY (void)
{
    PIN_InitConfig(P21_2, PIN_MODE_OUTPUT, 0);
    PIN_InitConfig(MOTOR1_P, PIN_MODE_OUTPUT, 0);
    PIN_InitConfig(P21_4, PIN_MODE_OUTPUT, 0);
    PIN_InitConfig(MOTOR2_P, PIN_MODE_OUTPUT, 0);

    //ATOM_PWM_InitConfig(MOTOR1_P, 0, MOTOR_FREQUENCY);
    ATOM_PWM_InitConfig(MOTOR1_N, 0, MOTOR_FREQUENCY);
    //ATOM_PWM_InitConfig(MOTOR2_P, 0, MOTOR_FREQUENCY);
    ATOM_PWM_InitConfig(MOTOR2_N, 0, MOTOR_FREQUENCY);

    //ATOM_PWM_SetDuty(MOTOR1_P, 10000, MOTOR_FREQUENCY);
    //ATOM_PWM_SetDuty(MOTOR2_P, 10000, MOTOR_FREQUENCY);
}

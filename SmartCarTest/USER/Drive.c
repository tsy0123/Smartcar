#include <LQ_PID.h>
#include <LQ_GPT12_ENC.h>
#include <Drive.h>
#include <TSY_WIFI.h>
pid_param_t pid_Drive;
pid_param_t pid_Left;
pid_param_t pid_Right;
int roadSpeed = 120;
int turnSpeed;
int time = 0;
float CenterERR = 0;
float CenterERR_Last = 0;
extern bool WIFI_show_Chart;
void TSY_MortorCtrl(sint32 motor1, sint32 motor2)
{
    if(motor1 > 0)
    {
        ATOM_PWM_SetDuty(MOTOR1_P, motor1, MOTOR_FREQUENCY);
        ATOM_PWM_SetDuty(MOTOR1_N, 0, MOTOR_FREQUENCY);
    }
    else
    {
        ATOM_PWM_SetDuty(MOTOR1_P, 0, MOTOR_FREQUENCY);
        ATOM_PWM_SetDuty(MOTOR1_N, -motor1, MOTOR_FREQUENCY);
    }
    if(motor2 > 0)
    {
        ATOM_PWM_SetDuty(MOTOR2_P, motor2, MOTOR_FREQUENCY);
        ATOM_PWM_SetDuty(MOTOR2_N, 0, MOTOR_FREQUENCY);
    }
    else
    {
        ATOM_PWM_SetDuty(MOTOR2_P, 0, MOTOR_FREQUENCY);
        ATOM_PWM_SetDuty(MOTOR2_N, -motor2, MOTOR_FREQUENCY);
    }

}
void TSY_Drive(pid_param_t *pidDrive, pid_param_t *pidLeft, pid_param_t *pidRight)
{
    short ENCLeft = ENC_GetCounter(ENC2_InPut_P33_7);
    short ENCRight = -ENC_GetCounter(ENC4_InPut_P02_8);
    short MotorLeft = 0;
    short MotorRight = 0;
    float speedSet = (float)roadSpeed;
    if((camERR.cam_finalCenterERR[0]>-4 && camERR.cam_finalCenterERR[0] < 4)\
            && (camERR.cam_finalCenterERR[1]>-4 && camERR.cam_finalCenterERR[1] < 4)\
            && (camERR.cam_finalCenterERR[2]>-4 && camERR.cam_finalCenterERR[2] < 4))
        speedSet = roadSpeed * 1.2;
    else
        speedSet = (float)roadSpeed;
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
        MotorLeft = pidLeft->out;
        MotorRight = pidRight->out;
        if(MotorLeft>5000) MotorLeft = 5000;
        if(MotorLeft<-5000) MotorLeft = -5000;
        if(MotorRight>5000) MotorRight = 5000;
        if(MotorRight<-5000) MotorRight = -5000;
        TSY_MortorCtrl(MotorLeft,MotorRight);
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
                MotorLeft = pidLeft->out;
                MotorRight = pidRight->out;
                if(MotorLeft>5000) MotorLeft = 5000;
                if(MotorLeft<-5000) MotorLeft = -5000;
                if(MotorRight>5000) MotorRight = 5000;
                if(MotorRight<-5000) MotorRight = -5000;
                TSY_MortorCtrl(MotorLeft,MotorRight);
                break;
            case 2:
                pidSetTarget(pidLeft, (float)roadSpeed);
                pidSetTarget(pidRight, -(float)roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                MotorLeft = pidLeft->out;
                MotorRight = pidRight->out;
                if(MotorLeft>5000) MotorLeft = 5000;
                if(MotorLeft<-5000) MotorLeft = -5000;
                if(MotorRight>5000) MotorRight = 5000;
                if(MotorRight<-5000) MotorRight = -5000;
                TSY_MortorCtrl(MotorLeft,MotorRight);
                break;
            case 1:
                pidSetTarget(pidLeft, -(float)roadSpeed);
                pidSetTarget(pidRight, (float)roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                MotorLeft = pidLeft->out;
                MotorRight = pidRight->out;
                if(MotorLeft>5000) MotorLeft = 5000;
                if(MotorLeft<-5000) MotorLeft = -5000;
                if(MotorRight>5000) MotorRight = 5000;
                if(MotorRight<-5000) MotorRight = -5000;
                TSY_MortorCtrl(MotorLeft,MotorRight);
                break;
            case 4:
                pidSetTarget(pidLeft, -(float)roadSpeed);
                pidSetTarget(pidRight, (float)-roadSpeed);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                MotorLeft = pidLeft->out;
                MotorRight = pidRight->out;
                if(MotorLeft>5000) MotorLeft = 5000;
                if(MotorLeft<-5000) MotorLeft = -5000;
                if(MotorRight>5000) MotorRight = 5000;
                if(MotorRight<-5000) MotorRight = -5000;
                TSY_MortorCtrl(MotorLeft,MotorRight);
                break;
            default:
                pidSetTarget(pidLeft, 0);
                pidSetTarget(pidRight, 0);
                PidLocCtrl(pidLeft, ENCLeft);
                PidLocCtrl(pidRight, ENCRight);
                MotorLeft = pidLeft->out;
                MotorRight = pidRight->out;
                if(MotorLeft>5000) MotorLeft = 5000;
                if(MotorLeft<-5000) MotorLeft = -5000;
                if(MotorRight>5000) MotorRight = 5000;
                if(MotorRight<-5000) MotorRight = -5000;
                TSY_MortorCtrl(MotorLeft,MotorRight);
                break;
        }
    }
}

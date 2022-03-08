#include <LQ_CAMERA.h>
#include "image.h"
#include <stdint.h>
#include <include.h>
#include <Main.h>
//直径64mm，周长201.1mm，1.5m/s速度，轮子转7.46圈,编码器输出764
extern int roadSpeed;   //(1.5/20.11/10.0*1024)
extern int turnSpeed;
#define turnLimit 3

//电机频率
#define MOTOR_FREQUENCY    10000

//电机PWM 宏定义
#define MOTOR1_P          P21_2
#define MOTOR1_N          IfxGtm_ATOM0_1_TOUT54_P21_3_OUT

#define MOTOR2_P          P21_4
#define MOTOR2_N          IfxGtm_ATOM0_3_TOUT56_P21_5_OUT


#define MOTOR3_P          IfxGtm_ATOM0_7_TOUT64_P20_8_OUT
#define MOTOR3_N          IfxGtm_ATOM0_3_TOUT56_P21_5_OUT


#define MOTOR4_P          IfxGtm_ATOM0_2_TOUT55_P21_4_OUT
#define MOTOR4_N          IfxGtm_ATOM0_1_TOUT54_P21_3_OUT


#define ATOMSERVO1       IfxGtm_ATOM2_0_TOUT32_P33_10_OUT
#define ATOMSERVO2       IfxGtm_ATOM2_5_TOUT35_P33_13_OUT

void TSY_MortorCtrl(sint32 motor1, sint32 motor2);
void TSY_Drive(pid_param_t *pidDrive, pid_param_t *pidLeft, pid_param_t *pidRight);
void MotorInit_TSY (void);
extern pid_param_t pid_Drive;
extern pid_param_t pid_Left;
extern pid_param_t pid_Right;

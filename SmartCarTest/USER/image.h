#ifndef IMAGE_H_
#define IMAGE_H_
#include <LQ_CAMERA.h>

#include <stdint.h>
#include <stdbool.h>
#include <LQ_GPIO.h>
#include <LQ_GPIO_LED.h>
#define mymax(a,b)  (((a)>(b))?(a):(b))
#define mymin(a,b)  (((a)<(b))?(a):(b))

//配置摄像头参数
#define MT9V03X_W               94                      // 图像宽度  范围1-752
#define MT9V03X_W_2             47
#define MT9V03X_W_2_3           62
#define MT9V03X_W_3             31
#define MT9V03X_W_4             23
#define MT9V03X_W_3_4           71

#define MT9V03X_H               60                      // 图像高度 范围1-480
#define MT9V03X_H_2             30
#define MT9V03X_H_3             20
#define MT9V03X_H_4             15
#define MT9V03X_H_6             10
#define MT9V03X_H_2_3           40
#define MT9V03X_H_5_6           50
#define TOTAL_POINT         MT9V03X_W*MT9V03X_H
#define EFFECTIVE_ROW           3                       //定义图片最上层的有效行

#define LOST_LINE_THRESHOLD     57                      //丢边行最大值
#define VALID_LINE_THRESHOLE    60-LOST_LINE_THRESHOLD  //有效行最小值
#define CROSS_EDGE_LEN          4                       //拟合十字路口两段直线时所需的最少点数

#define K_MAX_THRESHOLD         5                      //两有效点间最大斜率

//17th
#define roadK                   (0.2)                  //直赛道K
#define roadB                   (42)                 //直赛道B
#define SingleLineLeanAveERR_MAX 15                     //一边丢线，未丢线一边最大平均倾斜误差
#define SingleLineLeanK          20                     //一边丢线，未丢线一边补偿系数(理论距离实际中线的距离)

#define MINRoadLen               4                      //最小路宽
#define AIM_LINE_SET             30                     //目标行

#define IS_WHITE_ROW_NUM         92
#define IS_BLACK_ROW_NUM         30

#define BOTTOM_LEFT              10
#define BOTTOM_RIGHT             84
//斑马线相关参数
#define ZEBRA_DETECT_START_ROW          30
#define ZEBRA_DETECT_END_ROW            50
#define GARAGE_OUT_LOST                 4
//三叉相关参数
#define FORK_DETECT_START_ROW           56
#define FORK_DETECT_END_ROW             30
#define FORK_DETECT_TURN_END_ROW        10
#define FORK_DETECT_LOST_WHITE_ROW      50
#define FORK_DETECT_LOST_TURN_ROW       40

//圆环相关参数
#define JUDGE_LEFT_RIGHT_START_ROW      30
#define RING_EDGE_DECREASE_START_ROW    30
#define RING_EDGE_DECREASE_END_ROW      55
#define RING_EDGE_INCREASE_START_ROW    30
#define RING_EDGE_INCREASE_END_ROW      58
#define RING_SUDDEN_CHANGE_START_ROW    20
#define RING_SUDDEN_CHANGE_END_ROW      35
#define RING_IN_MEND_START_ROW          50
#define RING_IN_MEND_END_ROW            18
#define LOST_PICTURE_NUM                0
#define RING_OUT_TURNPOINT_START_ROW    45
#define RING_OUT_TURNPOINT_END_ROW      20
#define RING_LOST_COUNT                 4
#define RING_OUT_DETECT_START_ROW       35
#define RING_OUT_DETECT_END_ROW         50

//十字相关参数
#define CROSS_DETECT_LINE_COUNT         15
#define CROSS_LOST_ROW_COUNT            9
#define CROSS_OUT_EXIST                 35
extern uint8_t mt9v03x_image[MT9V03X_W][MT9V03X_H];//原始图像
typedef struct
{
    volatile short Point_Left[MT9V03X_H];                //存放某一行的左边界点
    volatile short Point_Right[MT9V03X_H];               //存放某一行的右边界点
    volatile short Point_Center[MT9V03X_H];              //存放某一行的中心点

    volatile short White_Num[MT9V03X_H];                 //某一行的白点个数

    volatile bool Exist_Left[MT9V03X_H];                 //某一行左边边界点存在
    volatile bool Exist_Right[MT9V03X_H];                //某一行右边边界点存在
    volatile bool Exist_Center[MT9V03X_H];               //某一行中心点存在

    volatile bool Lost_Center;                           //是否丢了中线
    volatile bool Lost_Left;                             //是否丢了左线
    volatile bool Lost_Right;                            //是否丢了右线

    volatile char pad;


}imageLine_t;

typedef struct
{
    volatile short cam_finalCenterERR[3];//摄像头最终数据（中线偏差）
    volatile float K_cam;//归一化系数：摄像头
}camERR_t;


//extern variables
extern volatile imageLine_t imageLine;
extern volatile camERR_t camERR;
extern uint8_t bin_thr;
extern int White_Num_Left;
extern int White_Num_Right;
extern uint8_t find_ring_flag_Left;
extern uint8_t find_ring_Left;
extern uint8_t find_ring_flag_Right;
extern uint8_t find_ring_Right;
extern uint8_t isLeft;
extern uint8_t isRight;
extern volatile uint8_t AIM_LINE;
extern uint8_t flag_isLeft_ring;
extern uint8_t flag_isRight_ring;
extern uint8_t lock_zebra;
extern bool isLeftLineStraight;
extern bool isRightLineStraight;
extern uint8_t startTick;
extern uint8_t flag_crossLeft_find;
extern uint8_t flag_crossRight_find;
extern uint8_t crossRight_flag;
extern uint8_t crossLeft_flag;
extern uint8_t zebra_cross_count;
extern uint8_t garage_in;
extern short flag_garage_turn;
extern uint8_t garage_in_flag;
extern short isForkRoadTurnLeft;
//图像处理函数///////////////////////////////////////////////////////////////
//imageAPI
//bool isTopExistPoint(uint8_t dir);//顶部是否存在边界点判断
bool isWhite(short row, short line);//白点判断
bool isLeftPoint(short i, uint8_t j);//左边界点判断
bool isRightPoint(short i, uint8_t j);//右边界点判断
bool isEdgePoint(short i, uint8_t j);//边界点判断
void Get_White_Num(uint8_t mode);//得到某一行白点个数
void Get_Left_Right_Num(void);
void Get_White_Num_Left(void);
void Get_White_Num_Right(void);
//imageFind
//图像预处理
void get_bin_thr(void);//获取二值化阈值（平均阈值法）
//short GetOSTU(uint8_t tmImage[MT9V03X_W][MT9V03X_H]);//获取二值化阈值（大津法）
void gray2bin(void);//灰度图像二值化
void Pixle_Filter(void);//滤除二值化图像噪点
void left_right_Limit(void);//保证同一行左边界点一定在右边界点左边
void image_pre_processing(void);//将上面对图像预处理的函数打包
//DFS找边界点
void ImageProcessInit(void);//图像处理相关变量初始化
void trackDFS(void);//深度优先搜索——初次寻找左右边界点
void lineChangeLimit(void);//边界点不突变///////////////////
//三岔路口判断
void forkRoad_find(void);//找三岔路口
void forkRoad_in_filter(short select);//进入三岔，滤除一边的边界点
void forkRoad_mend(short select);//上面的不好用，直接存下来点，补线


//出入库判断
void garage_find(void);//出库直走后打角判断
void zebra_cross_detect(void);
void garage_in_mend(void);
//


//imageFilter
void doFilter(void);//将下面所有filter函数打包
void lostLine_Filter(void);//无效行过多则判断丢边线
void position_Filter(void);//位置不对(左边线在右边×)则滤去
void slope_Filter(void);//边界线斜率不对则滤去不对的边界点(这个函数要放在十字串道后面)
void singlePoint_Filter(void);//滤除单个点


//imageMend
void doMend(void);

//void centerChangeLimit(void);//中心点不突变///////////////////

//圆环
void track_boundary_detect(void);//单边丢线时判断赛道边界的斜率(do_mend)
void ring_in_Mend(void);
void ring_detect(void);
void ring_Check(uint8_t way);
void ring_LostCenter_mend(void);//出环丢线补线(do_mend)
void ring_out_turnPoint_filter_mend(void);//出环找拐点并滤除拐点上面的边界点
void ring_out_detect(void);
void link_Mend(void);

//十字
void crossFilter_tsy(void);
void crossDetect_tsy(void);
void crossDetect_Left_tsy(void);
void crossDetect_Right_tsy(void);
void crossout_mend(void);
void cross_Check(void);

//imageCheck
void mediumLineCheck(void);

//辅助处理函数
short road_Width_L(uint8_t row, uint8_t way);
short road_Width_R(uint8_t row, uint8_t way);
bool White_Black_White_detect(uint8_t row, uint8_t half);
bool isStraightLeft(uint8_t row);
bool isStraightRight(uint8_t row);
//imageProcess
void updateMediumLine(void);//更新中线打角
bool MediumLineCal(volatile short* final);//计算中线打角


#endif




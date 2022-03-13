#ifndef IMAGE_H_
#define IMAGE_H_
#include <LQ_CAMERA.h>

#include <stdint.h>
#include <stdbool.h>
#include <LQ_GPIO.h>
#include <LQ_GPIO_LED.h>
#define mymax(a,b)  (((a)>(b))?(a):(b))
#define mymin(a,b)  (((a)<(b))?(a):(b))

//��������ͷ����
#define MT9V03X_W               94                      // ͼ����  ��Χ1-752
#define MT9V03X_W_2             47
#define MT9V03X_W_2_3           62
#define MT9V03X_W_3             31
#define MT9V03X_W_4             23
#define MT9V03X_W_3_4           71

#define MT9V03X_H               60                      // ͼ��߶� ��Χ1-480
#define MT9V03X_H_2             30
#define MT9V03X_H_3             20
#define MT9V03X_H_4             15
#define MT9V03X_H_6             10
#define MT9V03X_H_2_3           40
#define MT9V03X_H_5_6           50
#define TOTAL_POINT         MT9V03X_W*MT9V03X_H
#define EFFECTIVE_ROW           3                       //����ͼƬ���ϲ����Ч��

#define LOST_LINE_THRESHOLD     57                      //���������ֵ
#define VALID_LINE_THRESHOLE    60-LOST_LINE_THRESHOLD  //��Ч����Сֵ
#define CROSS_EDGE_LEN          4                       //���ʮ��·������ֱ��ʱ��������ٵ���

#define K_MAX_THRESHOLD         5                      //����Ч������б��

//17th
#define roadK                   (0.2)                  //ֱ����K
#define roadB                   (42)                 //ֱ����B
#define SingleLineLeanAveERR_MAX 15                     //һ�߶��ߣ�δ����һ�����ƽ����б���
#define SingleLineLeanK          20                     //һ�߶��ߣ�δ����һ�߲���ϵ��(���۾���ʵ�����ߵľ���)

#define MINRoadLen               4                      //��С·��
#define AIM_LINE_SET             30                     //Ŀ����

#define IS_WHITE_ROW_NUM         92
#define IS_BLACK_ROW_NUM         30

#define BOTTOM_LEFT              10
#define BOTTOM_RIGHT             84
//��������ز���
#define ZEBRA_DETECT_START_ROW          30
#define ZEBRA_DETECT_END_ROW            50
#define GARAGE_OUT_LOST                 4
//������ز���
#define FORK_DETECT_START_ROW           56
#define FORK_DETECT_END_ROW             30
#define FORK_DETECT_TURN_END_ROW        10
#define FORK_DETECT_LOST_WHITE_ROW      50
#define FORK_DETECT_LOST_TURN_ROW       40

//Բ����ز���
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

//ʮ����ز���
#define CROSS_DETECT_LINE_COUNT         15
#define CROSS_LOST_ROW_COUNT            9
#define CROSS_OUT_EXIST                 35
extern uint8_t mt9v03x_image[MT9V03X_W][MT9V03X_H];//ԭʼͼ��
typedef struct
{
    volatile short Point_Left[MT9V03X_H];                //���ĳһ�е���߽��
    volatile short Point_Right[MT9V03X_H];               //���ĳһ�е��ұ߽��
    volatile short Point_Center[MT9V03X_H];              //���ĳһ�е����ĵ�

    volatile short White_Num[MT9V03X_H];                 //ĳһ�еİ׵����

    volatile bool Exist_Left[MT9V03X_H];                 //ĳһ����߽߱�����
    volatile bool Exist_Right[MT9V03X_H];                //ĳһ���ұ߽߱�����
    volatile bool Exist_Center[MT9V03X_H];               //ĳһ�����ĵ����

    volatile bool Lost_Center;                           //�Ƿ�������
    volatile bool Lost_Left;                             //�Ƿ�������
    volatile bool Lost_Right;                            //�Ƿ�������

    volatile char pad;


}imageLine_t;

typedef struct
{
    volatile short cam_finalCenterERR[3];//����ͷ�������ݣ�����ƫ�
    volatile float K_cam;//��һ��ϵ��������ͷ
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
//ͼ������///////////////////////////////////////////////////////////////
//imageAPI
//bool isTopExistPoint(uint8_t dir);//�����Ƿ���ڱ߽���ж�
bool isWhite(short row, short line);//�׵��ж�
bool isLeftPoint(short i, uint8_t j);//��߽���ж�
bool isRightPoint(short i, uint8_t j);//�ұ߽���ж�
bool isEdgePoint(short i, uint8_t j);//�߽���ж�
void Get_White_Num(uint8_t mode);//�õ�ĳһ�а׵����
void Get_Left_Right_Num(void);
void Get_White_Num_Left(void);
void Get_White_Num_Right(void);
//imageFind
//ͼ��Ԥ����
void get_bin_thr(void);//��ȡ��ֵ����ֵ��ƽ����ֵ����
//short GetOSTU(uint8_t tmImage[MT9V03X_W][MT9V03X_H]);//��ȡ��ֵ����ֵ����򷨣�
void gray2bin(void);//�Ҷ�ͼ���ֵ��
void Pixle_Filter(void);//�˳���ֵ��ͼ�����
void left_right_Limit(void);//��֤ͬһ����߽��һ�����ұ߽�����
void image_pre_processing(void);//�������ͼ��Ԥ����ĺ������
//DFS�ұ߽��
void ImageProcessInit(void);//ͼ������ر�����ʼ��
void trackDFS(void);//�������������������Ѱ�����ұ߽��
void lineChangeLimit(void);//�߽�㲻ͻ��///////////////////
//����·���ж�
void forkRoad_find(void);//������·��
void forkRoad_in_filter(short select);//���������˳�һ�ߵı߽��
void forkRoad_mend(short select);//����Ĳ����ã�ֱ�Ӵ������㣬����


//������ж�
void garage_find(void);//����ֱ�ߺ����ж�
void zebra_cross_detect(void);
void garage_in_mend(void);
//


//imageFilter
void doFilter(void);//����������filter�������
void lostLine_Filter(void);//��Ч�й������ж϶�����
void position_Filter(void);//λ�ò���(��������ұߡ�)����ȥ
void slope_Filter(void);//�߽���б�ʲ�������ȥ���Եı߽��(�������Ҫ����ʮ�ִ�������)
void singlePoint_Filter(void);//�˳�������


//imageMend
void doMend(void);

//void centerChangeLimit(void);//���ĵ㲻ͻ��///////////////////

//Բ��
void track_boundary_detect(void);//���߶���ʱ�ж������߽��б��(do_mend)
void ring_in_Mend(void);
void ring_detect(void);
void ring_Check(uint8_t way);
void ring_LostCenter_mend(void);//�������߲���(do_mend)
void ring_out_turnPoint_filter_mend(void);//�����ҹյ㲢�˳��յ�����ı߽��
void ring_out_detect(void);
void link_Mend(void);

//ʮ��
void crossFilter_tsy(void);
void crossDetect_tsy(void);
void crossDetect_Left_tsy(void);
void crossDetect_Right_tsy(void);
void crossout_mend(void);
void cross_Check(void);

//imageCheck
void mediumLineCheck(void);

//����������
short road_Width_L(uint8_t row, uint8_t way);
short road_Width_R(uint8_t row, uint8_t way);
bool White_Black_White_detect(uint8_t row, uint8_t half);
bool isStraightLeft(uint8_t row);
bool isStraightRight(uint8_t row);
//imageProcess
void updateMediumLine(void);//�������ߴ��
bool MediumLineCal(volatile short* final);//�������ߴ��


#endif




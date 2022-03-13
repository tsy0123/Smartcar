#include <image.h>
#include <mymath.h>
#include <stdint.h>
#include <stdio.h>
#include <LQ_UART.h>
#include <LQ_STM.h>
#include <TSY_WIFI.h>

uint8_t mt9v03x_image[MT9V03X_W][MT9V03X_H];//原始图像track_boundary_k_left

uint8_t bin_thr = 0;//二值化阈值
volatile imageLine_t imageLine;
volatile camERR_t camERR;

volatile int ave_err_max = 0;//用于调参
volatile uint8_t turnPoint_view_L = EFFECTIVE_ROW;
volatile uint8_t turnPoint_view_R = EFFECTIVE_ROW;

short flag_forkRoad_prefind = 0;
short flag_forkRoad_find = 0;
short flag_prefind_forkRoadTurnPoint = 0;
short flag_find_forkRoadTurnPoint = 0;
short flag_garage_turn = 0;
short flag_garage_detect = 1;
short flag_find_ringOutTurnPoint = 0;
uint8_t flag_fullcross_left = 0;
uint8_t flag_fullcross_right = 0;
float track_boundary_k_right[3] = { 0,0,0 };//记录丢左线 右边界斜率
float track_boundary_k_left[3] = { 0,0,0 };//记录单边丢右线 左边界斜率
float track_boundary_k_err[2] = { 0,0 };
short flag_more_SingleLineLeanK = 0;

short isLeftToForkRoad = 1;//三岔前左转容易进三岔右边; 三岔前右转容易进三岔左边
short isForkRoadTurnLeft = 0;//三岔转向方向 1 左转; 0 右转

/*****************************************************************
*Function: isWhite(*)
*Description: 判断是否为白点
*parameter: row 图像行
            line 图像列
*Return: true 白点
         false 黑点
*****************************************************************/
bool isWhite(short line, short row)
{
    //出界判断
    if (!(row >= 0 && row < MT9V03X_H && line >= 0 && line < MT9V03X_W))
        return false;

    //判断白点黑点
    if (Bin_Pixle[line][row])
        return true;
    else
        return false;
}


/*****************************************************************
*Function: isLeftPoint(*)
*Description: 判断是否为左边界点
*parameter: i 像素点行
            j 像素点列
*Return: true 是左边界点
         false 不是左边界点
*****************************************************************/
bool isLeftPoint(short i, uint8_t j)
{
    if (i < 2 || i >= MT9V03X_W - 2 || j >= MT9V03X_H)//图像边缘
        return false;
    //右边一定不能出现蓝布
    if (((!isWhite(i, j)) || (!isWhite(i + 1, j)) || (!isWhite(i + 2, j)) || (!isWhite(i + 3, j)) ))
        return false;
    //左边一定不能出现路
    if (isWhite(i - 1, j) || isWhite(i - 2, j) || isWhite(i - 3, j) || isWhite(i - 4, j))
        return false;

    return true;
}

/*****************************************************************
*Function: isRightPoint(*)
*Description: 判断是否为右边界点
*parameter: i 像素点行
            j 像素点列
*Return: true 是有边界点
         false 不是右边界点
*****************************************************************/
bool isRightPoint(short i, uint8_t j)
{
    if (i < 2 || i >= MT9V03X_W - 2 || j >= MT9V03X_H)//图像边缘
        return false;
    //左边一定不能出现蓝布
    if (((!isWhite(i, j)) || (!isWhite(i - 1, j)) || (!isWhite(i - 2, j)) || (!isWhite(i - 3, j))))
        return false;
    //右边一定不能出现路
    if (isWhite(i + 1, j) || isWhite(i + 2, j) || isWhite(i + 3, j) || isWhite(i + 4, j))
        return false;

    return true;
}

/*****************************************************************
*Function: isEdgePoint(*)
*Description: 仅用于九宫格搜索(在已搜索到边界点的条件下，找下个边界点的条件)
*parameter: i 像素点行
            j 像素点列
*Return: true 是边界点
         false 不是边界点
*****************************************************************/
bool isEdgePoint(short i, uint8_t j)
{
    if (j < 1 || j >= MT9V03X_H - 1 || i<2 || i>MT9V03X_W - 3)//图像边缘
        return false;
    else if (
        ((isWhite(i, j)))//本身是白色
        && ((!isWhite(i + 1, j)) || (!isWhite(i - 1, j)) || (!isWhite(i, j + 1))
            || (!isWhite(i, j - 1)))//上下左右至少有一个黑色
        )
        return true;
    else
        return false;
}

int White_Num_Left = 0;
int White_Num_Right = 0;
/*****************************************************************
*Function: Get_White_Num(uint8_t mode)
*Description: 得到白点个数
*parameter: *
*Return: *
*****************************************************************/
void Get_White_Num(uint8_t mode)
{
    short i, j;
    short white_num = 0;
    if(mode == 0)
    {
        for (j = 0; j < MT9V03X_H; j++)
        {
            for (i = 0; i < MT9V03X_W; i++)
            {
                if (Bin_Pixle[i][j])
                {
                    white_num++;
                }
            }
            imageLine.White_Num[j] = white_num;
            white_num = 0;
        }
    }
    if(mode == 1)
    {
        for (j = 0; j < MT9V03X_H; j++)
        {
            for (i = 0; i < MT9V03X_W / 2; i++)
            {
                if (Bin_Pixle[i][j])
                {
                    white_num++;
                }
            }
        }
        White_Num_Left = white_num;
    }
    if(mode == 2)
    {
        for (j = 0; j < MT9V03X_H; j++)
        {
            for (i = MT9V03X_W / 2; i < MT9V03X_W; i++)
            {
                if (Bin_Pixle[i][j])
                {
                    white_num++;
                }
            }

        }
        White_Num_Right = white_num;
    }
}

/* special for trackFind.c */
static short stackTopPos;//栈顶指针
static uint8_t roadPointStackX[TOTAL_POINT];//DFS堆栈存储寻找X坐标
static uint8_t roadPointStackY[TOTAL_POINT];//DFS堆栈存储寻找Y坐标
static bool isVisited[MT9V03X_W][MT9V03X_H];//像素点是否访问过

/*****************************************************************
*Function: get_bin_thr(*)
*Description: 获取二值化阈值（平均法）
*parameter: *
*Return: *
*****************************************************************/
void get_bin_thr(void)//获取二值化阈值（平均法）
{
    int i, j;
    int image_gray_sum = 0;
    for (i = 0; i < MT9V03X_H; i++)
    {
        for (j = 0; j < MT9V03X_W; j++)
        {
            image_gray_sum += mt9v03x_image[j][i];
        }
    }
    bin_thr = image_gray_sum / (MT9V03X_H * MT9V03X_W);
    //bin_thr = GetOSTU(mt9v03x_image);
}



/*****************************************************************
*Function: gray2bin(*)
*Description: 灰度图像二值化
*parameter: *
*Return: *
*****************************************************************/
void gray2bin(void)
{
    for (int i = 0; i < MT9V03X_W; i++)
    {
        for (int j = 0; j < MT9V03X_H; j++)
        {
            if (mt9v03x_image[i][j] > bin_thr)//白点 1；黑点 0
                Bin_Pixle[i][j] = true;
            else
                Bin_Pixle[i][j] = false;
        }
    }
}

/*****************************************************************
*Function: Pixle_Filter(*)
*Description: 滤除二值化图像噪点
              根据赛道白色和蓝色都是连续的进行过滤噪点
*parameter: *
*Return: *
*****************************************************************/
void Pixle_Filter(void)
{
    int nr; //行
    int nc; //列

    for (nr = 1; nr < MT9V03X_W - 1; nr++)
    {
        for (nc = 1; nc < MT9V03X_H - 1; nc = nc + 1)
        {
            if ((Bin_Pixle[nr][nc] == 0) && (Bin_Pixle[nr - 1][nc] + Bin_Pixle[nr + 1][nc] + Bin_Pixle[nr][nc + 1] + Bin_Pixle[nr][nc - 1] > 2))
            {
                Bin_Pixle[nr][nc] = true;
            }
            else if ((Bin_Pixle[nr][nc] == 1) && (Bin_Pixle[nr - 1][nc] + Bin_Pixle[nr + 1][nc] + Bin_Pixle[nr][nc + 1] + Bin_Pixle[nr][nc - 1] < 2))
            {
                Bin_Pixle[nr][nc] = false;
            }
        }
    }
}

/*****************************************************************
*Function: left_right_Limit(*)
*Description: 保证同一行左边界点一定在右边界点左边
*parameter: *
*Return: *
*****************************************************************/
void left_right_Limit(void)
{
    short i = 0;

    for (i = 1; i < MT9V03X_H - 1; i++)//从左上到右下
    {
        if (imageLine.Exist_Left[i] && imageLine.Exist_Right[i])
        {
            if (imageLine.Point_Left[i] > imageLine.Point_Right[i])
            {
                imageLine.Exist_Right[i] = false;
            }
        }
    }
}


//三叉相关
uint8_t fork_Point_row = 0;
uint8_t fork_Point_line = 0;
uint8_t fork_Ypoint_Left = 0;
uint8_t fork_Ypoint_Right = 0;

/*****************************************************************
*Function: forkRoad_find(*)
*Description: 三岔路口判断（左拐）
*             检测思路：
*             1、检测Y形路口，检测到则将右下拐点与顶端岔路连线
*             2、若未检测到，判断左下拐点是否存在，若存在，一般会在顶部拐点右侧检测到左线，滤除这些左线，并将右下角与顶端拐点连线
*             3、若12都未检测到，则检测是否丢中线，若丢中线，则检测顶端岔路，顶端岔路排布为白黑白，满足条件超过8行，则将右下与顶端拐点连线
*parameter: *
*Return: *
*****************************************************************/
void forkRoad_find(void)
{
    uint8_t i;
    uint8_t turnPoint_Left = 0;
    uint8_t turnPoint_Right = 0;
    for (i = FORK_DETECT_START_ROW; i > FORK_DETECT_END_ROW; i--)//从下往上
    {
        if ((imageLine.Exist_Left[i] && imageLine.Exist_Left[i+1] && imageLine.Exist_Left[i+2] && imageLine.Exist_Left[i+3])\
                || (imageLine.Exist_Right[i] && imageLine.Exist_Right[i+1] && imageLine.Exist_Right[i+2] && imageLine.Exist_Right[i+3]))////////////////////////////////////////////////////////////// && -> ||
        {
            if (imageLine.Exist_Left[i] && imageLine.Exist_Left[i+1] && imageLine.Exist_Left[i+2] && imageLine.Exist_Left[i+3])
                if((imageLine.Point_Left[i+2] > imageLine.Point_Left[i+1] && imageLine.Point_Left[i+2] <= imageLine.Point_Left[i+3]\
                        && imageLine.Point_Left[i+1] > imageLine.Point_Left[i]))
                    turnPoint_Left = i+2;
            if(imageLine.Exist_Right[i] && imageLine.Exist_Right[i+1] && imageLine.Exist_Right[i+2] && imageLine.Exist_Right[i+3])
                if((imageLine.Point_Right[i+2] < imageLine.Point_Right[i+1] && imageLine.Point_Right[i+2] >= imageLine.Point_Right[i+3]\
                        && imageLine.Point_Right[i+1] < imageLine.Point_Right[i]))
                    turnPoint_Right = i+2;
        }
        if (turnPoint_Left > 0 && turnPoint_Right > 0)
        {
            flag_forkRoad_prefind = 1;
            break;
        }
        else
            flag_forkRoad_prefind = 0;
    }
    if(isForkRoadTurnLeft)
    {
        if(flag_forkRoad_prefind)
        {
            for (i = MIN(turnPoint_Left,turnPoint_Right); i > FORK_DETECT_TURN_END_ROW; i--)
            {
                if(White_Black_White_detect(i,2))
                {
                    for(uint8_t j = i - 1; j > MIN(turnPoint_Left,turnPoint_Right) - 2; j--)
                    {
                        if(imageLine.White_Num[j -1] < imageLine.White_Num[j])
                        {
                            flag_forkRoad_prefind = 0;
                            return;
                        }
                    }
                    for(uint8_t j = 0; j < 93; j++)
                    {
                        if(isWhite(j, i) && !isWhite(j+1, i))
                        {
                            fork_Point_line = j+1;
                            break;
                        }
                    }
                    flag_forkRoad_prefind = 0;
                    fork_Point_row = i;
                    flag_forkRoad_find = 1;
                    fork_Ypoint_Left = turnPoint_Left;
                    fork_Ypoint_Right = turnPoint_Right;

                    //printf("%d %d %d %d \r\n",fork_Point_line,fork_Point_row,fork_Ypoint_Left,fork_Ypoint_Right);
                    break;
                }
            }
            if(i == 0)
            {
                flag_forkRoad_prefind = 0;
            }
        }
        else if(turnPoint_Left > 0)
        {
            for(i = turnPoint_Left; i > 0; i--)
            {
                if(imageLine.Exist_Left[i] && !imageLine.Exist_Left[i+1] && !imageLine.Exist_Left[i+2])
                {
                    if(White_Black_White_detect(i,2))
                    {
                        flag_forkRoad_prefind = 1;
                        break;
                    }
                }
            }
            if(flag_forkRoad_prefind)
            {
                uint8_t left_point = i;
                uint8_t isfork = 1;
                for(uint8_t j = left_point; j > 1; j--)
                {
                    if(imageLine.Exist_Left[j] && imageLine.Exist_Left[j-1] && imageLine.Point_Left[j-1]-imageLine.Point_Left[j]>=2)
                        continue;
                    else
                    {
                        isfork = 0;
                        break;
                    }
                }

                //三岔判断条件
                if (isfork)
                {
                    for(uint8_t j = left_point; j > 0; j--)
                    {
                        if(imageLine.Exist_Left[j])
                            imageLine.Exist_Left[j] = false;
                    }
                    flag_forkRoad_prefind = 0;
                    flag_forkRoad_find = 1;
                    fork_Point_row = left_point;
                    fork_Ypoint_Left = turnPoint_Left;
                    fork_Ypoint_Right = 59;
                    imageLine.Exist_Right[fork_Ypoint_Right] = true;
                    imageLine.Point_Right[fork_Ypoint_Right] = 93;
                }
                else
                {
                    flag_forkRoad_prefind = 0;
                }
            }
        }
        else if(imageLine.Lost_Right)
        {
            uint8_t isWBW = 0;
            if(!White_Black_White_detect(0,2) || imageLine.White_Num[0] > 60)
                return;
            for(i = 59; i > FORK_DETECT_LOST_WHITE_ROW; i--)
            {
                if(imageLine.White_Num[i]<IS_WHITE_ROW_NUM)
                    return;
            }
            for(i = FORK_DETECT_LOST_TURN_ROW; i > 0; i--)
            {
                if(White_Black_White_detect(i,2))
                {
                    if(isWBW == 0)
                    {
                        fork_Point_row = i;
                        for(uint8_t j = 2; j < 93; j++)
                            if(isWhite(j,i)&&!isWhite(j+1,i))
                            {
                                    fork_Point_line = j - 2;
                                    break;
                            }
                    }
                    isWBW++;
                    if(isWBW > 8)
                    {
                        fork_Ypoint_Left = 59;
                        fork_Ypoint_Right = 59;
                        flag_forkRoad_find = 1;
                        imageLine.Exist_Right[fork_Ypoint_Right] = true;
                        imageLine.Point_Right[fork_Ypoint_Right] = 93;
                        imageLine.Exist_Left[fork_Ypoint_Left] = true;
                        imageLine.Point_Left[fork_Ypoint_Left] = 0;
                        return;
                    }
                }
                else if(isWBW>0)
                    return;
            }
        }
    }
    else
    {
        if(flag_forkRoad_prefind)
        {
            for (i = MIN(turnPoint_Left,turnPoint_Right); i > FORK_DETECT_TURN_END_ROW; i--)
            {
                if(White_Black_White_detect(i,2))
                {
                    for(uint8_t j = i - 1; j > MIN(turnPoint_Left,turnPoint_Right) - 2; j--)
                    {
                        if(imageLine.White_Num[j -1] < imageLine.White_Num[j])
                        {
                            flag_forkRoad_prefind = 0;
                            return;
                        }
                    }
                    for(uint8_t j = 0; j < 93; j++)
                    {
                        if(isWhite(j, i) && !isWhite(j+1, i))
                        {
                            fork_Point_line = j+1;
                            break;
                        }
                    }
                    flag_forkRoad_prefind = 0;
                    fork_Point_row = i;
                    flag_forkRoad_find = 1;
                    fork_Ypoint_Left = turnPoint_Left;
                    fork_Ypoint_Right = turnPoint_Right;
                    //printf("%d %d %d %d \r\n",fork_Point_line,fork_Point_row,fork_Ypoint_Left,fork_Ypoint_Right);
                    break;
                }
            }
            if(i == 0)
            {
                flag_forkRoad_prefind = 0;
            }
        }
        else if(turnPoint_Right > 0)
        {
            for(i = turnPoint_Right; i > 0; i--)
            {
                if(imageLine.Exist_Right[i] && !imageLine.Exist_Right[i+1] && !imageLine.Exist_Right[i+2])
                {
                    if(White_Black_White_detect(i,2))
                    {
                        flag_forkRoad_prefind = 1;
                        break;
                    }
                }
            }
            if(flag_forkRoad_prefind)
            {
                uint8_t right_point = i;
                uint8_t isfork = 1;
                for(uint8_t j = right_point; j > 1; j--)
                {
                    if(imageLine.Exist_Right[j] && imageLine.Exist_Right[j-1] && imageLine.Point_Right[j-1]-imageLine.Point_Right[j]>=2)
                        continue;
                    else
                    {
                        isfork = 0;
                        break;
                    }
                }

                //三岔判断条件
                if (isfork)
                {
                    for(uint8_t j = right_point; j > 0; j--)
                    {
                        if(imageLine.Exist_Right[j])
                            imageLine.Exist_Right[j] = false;
                    }
                    flag_forkRoad_prefind = 0;
                    flag_forkRoad_find = 1;
                    fork_Point_row = right_point;
                    fork_Ypoint_Left = turnPoint_Right;
                    fork_Ypoint_Right = 59;
                    imageLine.Exist_Left[fork_Ypoint_Right] = true;
                    imageLine.Point_Left[fork_Ypoint_Right] = 0;
                }
                else
                {
                    flag_forkRoad_prefind = 0;
                }
            }
        }
        else if(imageLine.Lost_Left)
        {
            uint8_t isWBW = 0;
            if(!White_Black_White_detect(0,2) || imageLine.White_Num[0] > 60)
                return;
            for(i = 59; i > FORK_DETECT_LOST_WHITE_ROW; i--)
            {
                if(imageLine.White_Num[i]<IS_WHITE_ROW_NUM)
                    return;
            }
            for(i = FORK_DETECT_LOST_TURN_ROW; i > 0; i--)
            {
                if(White_Black_White_detect(i,2))
                {
                    if(isWBW == 0)
                    {
                        fork_Point_row = i;
                        for(uint8_t j = 2; j < 93; j++)
                            if(isWhite(j,i)&&!isWhite(j+1,i))
                            {
                                    fork_Point_line = j - 2;
                                    break;
                            }
                    }
                    isWBW++;
                    if(isWBW > 8)
                    {
                        fork_Ypoint_Left = 59;
                        fork_Ypoint_Right = 59;
                        flag_forkRoad_find = 1;
                        imageLine.Exist_Right[fork_Ypoint_Right] = true;
                        imageLine.Point_Right[fork_Ypoint_Right] = 93;
                        imageLine.Exist_Left[fork_Ypoint_Left] = true;
                        imageLine.Point_Left[fork_Ypoint_Left] = 0;
                        return;
                    }
                }
                else if(isWBW>0)
                    return;
            }
        }
    }
}

/*****************************************************************
*Function: forkRoad_in_filter(*)
*Description: 进入三岔，滤除一边的边界点
*parameter: select 0,滤除右边界点；1,滤除左边界点
*Return: *
*****************************************************************/
void forkRoad_in_filter(short select)
{
    short i = 0;

    if (select == 0)
    {
        //imageLine.Lost_Right = true;
        for (i = fork_Ypoint_Left; i > EFFECTIVE_ROW; i--)//从下往上
        {
            if (imageLine.Exist_Left[i])
                imageLine.Exist_Left[i] = false;
        }
    }
    else if (select == 1)
    {
        //imageLine.Lost_Left = true;
        for (i = fork_Ypoint_Right; i > EFFECTIVE_ROW; i--)//从下往上
        {
            if (imageLine.Exist_Right[i])
                imageLine.Exist_Right[i] = false;
        }
    }
}

/*****************************************************************
*Function: forkRoad_mend(*)
*Description: 三岔补边界点
*parameter: select 0,补左边界点 右转；1,补右边界点 左转 只写了左转
*Return: *
*****************************************************************/
void forkRoad_mend(short select)
{
    uint8_t i = 0;

    //滤除另一边的边界点
    forkRoad_in_filter(select);
    //flag_crossLeft_find=0;
    //flag_crossRight_find=0;
    //crossRight_flag=0;
    //crossLeft_flag=0;
    //三岔补线
    if (select == 0)
    {
        //补左边界点（图像右半部分，右转）
        imageLine.Lost_Right = false;
        imageLine.Lost_Left = false;
        imageLine.Lost_Center = false;

        short x2 = imageLine.Point_Left[fork_Ypoint_Left];
        short y2 = fork_Ypoint_Left;
        short x1 = fork_Point_line;
        short y1 = fork_Point_row;
        short x3 = x2 - x1;
        short y3 = y2 - y1;
        short k;
        for (k = y1; k <= y2; k++)
        {
            imageLine.Exist_Left[k] = true;
            imageLine.Point_Left[k] = (short)(x3 * (k - y1) / y3 + x1);
        }
        uint8_t count = 0;
        short MendBasis_left[2][5];
        float k_left, b_left;

        for (i = 0; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Left[i])
            {
                MendBasis_left[0][count] = i;
                MendBasis_left[1][count] = imageLine.Point_Left[i];
                count++;
            }
            if (count == 5)
                break;
        }
        if (count == 5)//有5个点即可开始补线
        {
            leastSquareMethod(MendBasis_left[0], MendBasis_left[1], 5, &k_left, &b_left);

            //开始补线
            for (i = 0; i < fork_Point_row; i++)
            {
                if (!imageLine.Exist_Left[i])
                {
                    imageLine.Point_Left[i] = getLineValue(i, k_left, b_left);
                    imageLine.Exist_Left[i] = true;
                }

            }
        }
        flag_forkRoad_find = 0;
    }
    else if (select == 1)
    {
        //补右边界点（图像左半部分，左转）
        imageLine.Lost_Left = false;
        imageLine.Lost_Right = false;
        imageLine.Lost_Center = false;

        short x2 = imageLine.Point_Right[fork_Ypoint_Right];
        short y2 = fork_Ypoint_Right;
        short x1 = fork_Point_line;
        short y1 = fork_Point_row;
        short x3 = x2 - x1;
        short y3 = y2 - y1;
        short k;
        for (k = y1; k <= y2; k++)
        {
            imageLine.Exist_Right[k] = true;
            imageLine.Point_Right[k] = (short)(x3 * (k - y1) / y3 + x1);
        }

        uint8_t count = 0;
        short MendBasis_right[2][5];
        float k_right, b_right;

        for (i = 0; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Right[i])
            {
                MendBasis_right[0][count] = i;
                MendBasis_right[1][count] = imageLine.Point_Right[i];
                count++;
            }
            if (count == 5)
                break;
        }
        if (count == 5)//有5个点即可开始补线
        {
            leastSquareMethod(MendBasis_right[0], MendBasis_right[1], 5, &k_right, &b_right);

            //开始补线
            for (i = 0; i < fork_Point_row; i++)
            {
                if (!imageLine.Exist_Right[i])
                {
                    imageLine.Point_Right[i] = getLineValue(i, k_right, b_right);
                    imageLine.Exist_Right[i] = true;
                }

            }
        }
        flag_forkRoad_find = 0;
    }

}

/*****************************************************************
*Function: garage_find(*)
*Description: 出库直走后打角判断
*parameter: *
*Return: *
*****************************************************************/
uint8_t garage_lost = 0;
void garage_find(void)
{
    uint8_t i = 0;
    static short black_point_count = 0;
    static short black_line_count = 0;
    uint8_t flag_mend = 0;
    if(flag_garage_turn == 0)
    {
        for (i = MT9V03X_H - 1; i > 35; i--)
        {
            if(imageLine.White_Num[i]>IS_WHITE_ROW_NUM)
                black_line_count++;

            if (black_line_count > 7)//连续5行全黑
            {
                flag_garage_turn = 1;
                //flag_garage_detect = 0;//检测到一次之后就不检测了
                black_point_count = 0;
                black_line_count = 0;
                break;
            }
        }
    }
    else if(flag_garage_turn == 1)
    {
        for(i = 15; i < 40; i++)
        {
            if(imageLine.Exist_Left[i] && imageLine.Point_Left[i]>47)
            {
                flag_mend = 1;
                break;
            }
        }
        if (flag_mend)//连续5行全黑
        {
            flag_garage_turn = 2;
            //flag_garage_detect = 0;//检测到一次之后就不检测了
            garage_lost = 0;
        }
    }
}

uint8_t zebra_cross_count = 0;  //斑马线有效个数
uint8_t lock_zebra = 0; //是否不检测斑马线
uint8_t is_zebra = 0;   //判断是否存在斑马线
uint8_t garage_in = 0;  //入库判断标志
/*****************************************************************
*Function: zebra_cross_detect(*)
*Description: 检测斑马线
*             检测白黑（WB）和黑白（BW）排序，检测到各自++，达到7个以上即判断为斑马线
*parameter: *
*Return: *
*****************************************************************/
void zebra_cross_detect(void)
{
    uint8_t BW = 0, WB = 0;
    uint8_t i,j;
    is_zebra = 0;
    for(i = ZEBRA_DETECT_START_ROW; i < ZEBRA_DETECT_END_ROW; i++)
    {
        for(j = 0; j < MT9V03X_W - 1; j++)
        {
            if(isWhite(j,i) && !isWhite(j+1,i))
                WB++;
            else if(!isWhite(j,i) && isWhite(j+1,i))
                BW++;
            if(WB > 6 && BW > 6)
            {
                is_zebra = 1;
                crossLeft_flag = 0;
                crossRight_flag = 0;
                flag_crossLeft_find = 0;
                flag_crossRight_find = 0;
                isLeft = 0;
                isRight = 0;
                find_ring_flag_Left = 0;
                find_ring_flag_Right = 0;
                flag_isRight_ring = 0;
                flag_isLeft_ring = 0;
                break;
            }
        }
        if(WB > 6 && BW > 6)
        {
            if(!lock_zebra)
            {
                zebra_cross_count++;
                lock_zebra = 1;
            }
            break;
        }
        WB=0;
        BW=0;
    }
    if(zebra_cross_count == 2)
    {
        garage_in = 1;
    }
}

uint8_t garage_in_flag = 0;
/*****************************************************************
*Function: garage_in_mend(*)
*Description: 入库补线
*          直接给偏差
*parameter: *
*Return: *
*****************************************************************/
void garage_in_mend(void)
{
    if(garage_in_flag == 0)
    {
        imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 65;
        imageLine.Lost_Center = false;
            if(imageLine.White_Num[30] < 20)
            {
                garage_in_flag++;
            }
    }
    else if(garage_in_flag == 1)
    {
        for(uint8_t i = MT9V03X_H_2; i < MT9V03X_H_2_3; i++)//检测黑点数，检测到停下
        {
            if(imageLine.White_Num[i] < 10)
            {
                manControl = 1;
                break;
            }
        }
    }
}




bool isLeftLineStraight = true;//左线是否为直线
bool isRightLineStraight = true;//右线是否为直线
static bool isLeftRightLineExist = true;//降标准检测左右边界是否存在
uint8_t find_ring_Left = 0; //左环进环标志
uint8_t find_ring_Right = 0;//右环进环标志
uint8_t find_ring_flag_Right = 0;//右环检测标志
uint8_t find_ring_flag_Left = 0;//左环检测标志
uint8_t flag_isLeft_ring = 0;//左环环内标志，出环清除
uint8_t flag_isRight_ring = 0;//右环环内标志，出环清除
uint8_t isLeft = 0, isRight = 0;//开始检测左右环标志
uint8_t startTick = 0;//开始计时，定时清除环内标志
static uint8_t ring_in_mend_count = 0; //没检测到入环补线拐点计数
static uint8_t ring_in_lost_left = 0;  //没检测到入左环补线标志
static uint8_t ring_in_lost_right = 0; //没检测到入右环补线标志
static uint8_t ring_out_turnPoint_row = EFFECTIVE_ROW;
short ring_out_turnPoint_line = 0;

/*出环检测
 * 检测白黑白
 */
void ring_out_detect(void)
{
    uint8_t i;
   if ((flag_isLeft_ring == 1 && find_ring_Left == 0)||(flag_isRight_ring == 1 && find_ring_Right == 0))
   {
       for(i = RING_OUT_DETECT_START_ROW; i < RING_OUT_DETECT_END_ROW; i++)
       {
           if(flag_isLeft_ring)
              if((isLeftLineStraight || isRightLineStraight) && White_Black_White_detect(i,0))
              {
                  flag_isRight_ring = 0;
                  isRight = 0;
                  flag_isLeft_ring = 0;
                  isLeft = 0;
                  crossLeft_flag = 0;
                  crossRight_flag = 0;
                  flag_crossLeft_find = 0;
                  flag_crossRight_find = 0;
                  break;
              }
           if(flag_isRight_ring)
             if((isLeftLineStraight || isRightLineStraight) && White_Black_White_detect(i,1))
             {
                 flag_isRight_ring = 0;
                 isRight = 0;
                 flag_isLeft_ring = 0;
                 isLeft = 0;
                 crossLeft_flag = 0;
                 crossRight_flag = 0;
                 flag_crossLeft_find = 0;
                 flag_crossRight_find = 0;
                 break;
             }
       }
   }
}

/*****************************************************************
*Function: ring_detect(*)
*Description: 检测圆环
* *          右环为例
*              |   |
*  *判断右环->   |   |/-----\
*              |       --  \
*  *阶段3->     |      /  \ |
*  *阶段2->     |      \  / |
*  *阶段1->     |       --  /
*  *开始判断->   |   |\-----/
*              |   |
*parameter: *
*Return: *
*****************************************************************/
void ring_detect(void)
{
    if(flag_isLeft_ring || flag_isRight_ring)
        return;
    uint8_t i;
    short road_Bottom_Left = 0, road_Bottom_Right = 0;
    int count;

    count=0;
    //找拐点判断左圆环还是右圆环，判断方法为 一边为 白->黑->白 ，另一边为直线
    if(!isLeft && !isRight)
    {
        if(isRightLineStraight)
        {

            for(i = JUDGE_LEFT_RIGHT_START_ROW; i < MT9V03X_H; i++)
            {
                if(White_Black_White_detect(i,0))
                {
                    isLeft = 1;
                    //printf("LL\r\n");
                    break;
                }
            }
        }
        if(isLeftLineStraight)
        {
            for(i = JUDGE_LEFT_RIGHT_START_ROW; i < MT9V03X_H; i++)
            {
                if(White_Black_White_detect(i,1))
                {
                    isRight = 1;
                    //printf("RR");
                    break;
                }
            }
        }
    }
    if(isLeft && isRight)
    {
        isLeft=0;
        isRight=0;
    }
    /*检测思路
            1、检测路宽突变
            2、检测路宽变窄
            3、检测路宽变宽
            4、检测拐点
            5、转弯
            */
    if(isLeft)
    {
        isRight=0;
        switch(find_ring_flag_Left)
        {
            case 0:
                for(i = JUDGE_LEFT_RIGHT_START_ROW; i < MT9V03X_H; i++)
                {
                    if(road_Width_L(i-2, 0) > road_Width_L(i, 0) + 4 && road_Width_L(i, 0)!=0)
                    {
                        find_ring_flag_Left++;
                        break;
                    }
                }
                break;
            case 1:
                for(i = RING_EDGE_DECREASE_START_ROW; i < RING_EDGE_DECREASE_END_ROW; i++)
                {
                    if(imageLine.Exist_Left[i] && imageLine.Exist_Left[i+1])
                        if(imageLine.Point_Left[i]-imageLine.Point_Left[i+1]>1)
                            count++;
                    if(count > 1)
                    {
                        find_ring_flag_Left++;
                        break;
                    }
                }
                count=0;
                break;
            case 2:
                for(i = RING_EDGE_INCREASE_START_ROW; i < RING_EDGE_INCREASE_END_ROW; i++)
                {
                    if(imageLine.Exist_Left[i] && imageLine.Exist_Left[i+1] && imageLine.Exist_Left[i+2])
                        if(imageLine.Point_Left[i] < imageLine.Point_Left[i+1]&& imageLine.Point_Left[i+1] < imageLine.Point_Left[i+2])
                                count++;
                    if(count > 0)
                    {
                        find_ring_flag_Left++;
                        break;
                    }
                }
                break;
            case 3:

                for(i = RING_SUDDEN_CHANGE_START_ROW; i < RING_SUDDEN_CHANGE_END_ROW; i++)
                {
                    if(road_Width_L(i + 1, 0) > road_Width_L(i, 0) + 8 && road_Width_L(i, 0)!=0)
                    {
                        road_Bottom_Left = i;
                        break;
                    }
                }
                if(road_Bottom_Left > RING_SUDDEN_CHANGE_START_ROW - 1 )
                {
                    find_ring_Left=1;
                    flag_isLeft_ring=1;
                    find_ring_Right=0;
                    flag_isRight_ring=0;
                    isLeft = 0;
                    startTick = 1;
                }
                break;
        }
    }
    if(isRight)
    {
        isLeft=0;
        switch(find_ring_flag_Right)
        {
            case 0:
                for(i = JUDGE_LEFT_RIGHT_START_ROW; i < MT9V03X_H; i++)
                {
                    if(road_Width_R(i-2,0)>road_Width_R(i,0)+4 && road_Width_R(i,0)!=0)
                    {
                        find_ring_flag_Right++;
                        //printf("R0");
                        break;
                    }
                }
                break;
            case 1:
                for(i = RING_EDGE_DECREASE_START_ROW; i < RING_EDGE_DECREASE_END_ROW; i++)
                {
                    if(imageLine.Exist_Right[i]&&imageLine.Exist_Right[i+1])
                        if(imageLine.Point_Right[i+1]-imageLine.Point_Right[i]>1)
                            count++;
                        if(count>1)
                        {
                            find_ring_flag_Right++;
                            //printf("R1");
                            break;
                        }
                }
                break;
            case 2:
                for(i = RING_EDGE_INCREASE_START_ROW; i < RING_EDGE_INCREASE_END_ROW; i++)
                {
                    if(imageLine.Exist_Right[i] && imageLine.Exist_Right[i+1] && imageLine.Exist_Right[i+2])
                        if(imageLine.Point_Right[i] > imageLine.Point_Right[i+1]&& imageLine.Point_Right[i+1] > imageLine.Point_Right[i+2])
                            count++;
                    if(count>0)
                    {
                        find_ring_flag_Right++;
                        //printf("R2");
                        break;
                    }
                }
                find_ring_flag_Right++;
                break;
            case 3:
                for(i = RING_SUDDEN_CHANGE_START_ROW; i < RING_SUDDEN_CHANGE_END_ROW; i++)
                {
                    if(road_Width_R(i+1,0)>road_Width_R(i,0)+8 && road_Width_R(i,0)!=0)
                    {
                        road_Bottom_Right = i;
                        break;
                    }
                }

                if(road_Bottom_Right > RING_SUDDEN_CHANGE_START_ROW - 1)
                {
                    find_ring_Right=1;
                    flag_isRight_ring=1;
                    find_ring_Left=0;
                    flag_isLeft_ring=0;
                    isRight = 0;
                    startTick = 1;
                }
                break;
        }
    }
}

/*****************************************************************
*Function: ring_in_Mend(*)
*Description: 入环补线
*parameter: *
*Return: *
*****************************************************************/
void ring_in_Mend(void)
{
    uint8_t i;
    uint8_t road_Bottom_Left = 0, road_Bottom_Right = 0;
    //补线思路
    //检测路宽突变，在下方时先检测白黑白，检测到补线，未检测到强制给偏差，3帧以上则判断入环完成
    if(find_ring_Left)
    {
        for(i = RING_IN_MEND_START_ROW; i > RING_IN_MEND_END_ROW; i--)//从下往上
        {
            if(road_Width_L(i+1,0)>road_Width_L(i,0) + 8 && road_Width_L(i,0)!=0 && i <40)
            {
                road_Bottom_Left = i;
                break;
            }
            if(White_Black_White_detect(i,1))
            {
                road_Bottom_Left = i;
                break;
            }
        }
        if(road_Bottom_Left == 0)
        {
            if(!imageLine.Lost_Right)
                ring_in_mend_count++;
            if(ring_in_mend_count > LOST_PICTURE_NUM)
            {
                find_ring_Left=0;
                find_ring_flag_Left=0;
                ring_in_mend_count=0;
            }
            ring_in_lost_left = 1;
            return;
        }
        imageLine.Lost_Left = true;
        imageLine.Lost_Right = false;
        for(i = 0; i < MT9V03X_H; i++)
        {
            imageLine.Exist_Left[i] = false;
            imageLine.Exist_Right[i] = false;
        }
        for(i=MT9V03X_W - 10; i>10; i--)
        {
            if(isWhite(i+1,road_Bottom_Left)&&!isWhite(i,road_Bottom_Left))
            {
                imageLine.Point_Right[road_Bottom_Left] = i;
                imageLine.Exist_Right[road_Bottom_Left] = true;
                break;
            }
        }
        if(!imageLine.Exist_Right[road_Bottom_Right])
        {
            imageLine.Point_Right[road_Bottom_Right] = 47;
            imageLine.Exist_Right[road_Bottom_Right] = true;
        }
        //和拐点连线
        short x1 = imageLine.Point_Right[road_Bottom_Left];
        short y1 = road_Bottom_Left;
        short x2 = MT9V03X_W - 1;
        short y2 = MT9V03X_H - 1;
        short k;
        for (k = y1; k <= y2; k++)
        {
            imageLine.Exist_Right[k] = true;
            imageLine.Point_Right[k] = (short)((x2 - x1) * (k - y1) / (y2 - y1) + x1);
        }

        //补全上方的线
        uint8_t count = 0;
        short MendBasis_right[2][5];
        float k_right, b_right;

        for (i = road_Bottom_Left; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Right[i])
            {
                MendBasis_right[0][count] = i;
                MendBasis_right[1][count] = imageLine.Point_Right[i];
                count++;
            }
            if (count == 5)
                break;
        }
        if (count == 5)//有5个点即可开始补线
        {
            leastSquareMethod(MendBasis_right[0], MendBasis_right[1], 5, &k_right, &b_right);

            if(ABS(k_right) > 3)
            {
                int *p;
                p = (int *)(&k_right);
                *p = *p >> 2;
                for (i = 0; i < road_Bottom_Left; i++)
                {
                    if (!imageLine.Exist_Left[i])
                    {
                        imageLine.Point_Left[i] = getLineValueX(i, MendBasis_right[0][0], MendBasis_right[1][0], k_right);
                        imageLine.Exist_Left[i] = true;
                    }

                }
                return;
            }
            //开始补线
            for (i = 0; i < road_Bottom_Left; i++)
            {
                if (!imageLine.Exist_Right[i])
                {
                    imageLine.Point_Right[i] = getLineValue(i, k_right, b_right);
                    imageLine.Exist_Right[i] = true;
                }

            }
        }
        /*imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 30;
        imageLine.Lost_Center = false;*/
    }
    else if(find_ring_Right)
    {
        for(i = RING_IN_MEND_START_ROW; i > RING_IN_MEND_END_ROW; i--)
        {
            //printf("%d %d\r\n",road_Width_R(i+1,2),road_Width_R(i,2));

            if(road_Width_R(i+1,0)>road_Width_R(i,0) + 8 && road_Width_R(i,0)!=0 && i <40)
            {
                road_Bottom_Right = i;
                break;
            }
            if(White_Black_White_detect(i,0))
            {
                road_Bottom_Right = i;
                break;
            }
        }
        if(road_Bottom_Right==0)
        {
            if(!imageLine.Lost_Left)
            ring_in_mend_count++;
            if(ring_in_mend_count > LOST_PICTURE_NUM)
            {
                find_ring_Right=0;
                find_ring_flag_Right=0;
                ring_in_mend_count=0;

            }
            ring_in_lost_right = 1;
            return;
        }

        imageLine.Lost_Right = true;
        imageLine.Lost_Left = false;
        for(i = 0; i < MT9V03X_H; i++)
        {
            imageLine.Exist_Left[i] = false;
            imageLine.Exist_Right[i] = false;
        }
        for(i = 10; i < MT9V03X_W - 10; i++)
        {
            if(isWhite(i,road_Bottom_Right) && !isWhite(i+1,road_Bottom_Right))
            {
                imageLine.Point_Left[road_Bottom_Right] = i;
                imageLine.Exist_Left[road_Bottom_Right] = true;
                break;
            }
        }
        if(!imageLine.Exist_Left[road_Bottom_Right])
        {
            imageLine.Point_Left[road_Bottom_Right] = 47;
            imageLine.Exist_Left[road_Bottom_Right] = true;
        }
        short x1 = imageLine.Point_Left[road_Bottom_Right];
        short y1 = road_Bottom_Right;
        short x2 = 0;
        short y2 = MT9V03X_H - 1;
        short k;
        for (k = y1; k <= y2; k++)
        {
            imageLine.Exist_Left[k] = true;
            imageLine.Point_Left[k] = (short)((x2 - x1) * (k - y1) / (y2 - y1) + x1);
        }
        uint8_t count = 0;
        short MendBasis_left[2][5];
        float k_left, b_left;

        for (i = road_Bottom_Right; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Left[i])
            {
                MendBasis_left[0][count] = i;
                MendBasis_left[1][count] = imageLine.Point_Left[i];
                count++;
            }
            if (count == 5)
                break;
        }
        if (count == 5)//有5个点即可开始补线
        {
            leastSquareMethod(MendBasis_left[0], MendBasis_left[1], 5, &k_left, &b_left);

            if(ABS(k_left)>3)
            {
                int *p;
                p = (int *)(&k_left);
                *p = *p >> 2;
                for (i = 0; i < road_Bottom_Right; i++)
                {
                    if (!imageLine.Exist_Left[i])
                    {
                        imageLine.Point_Left[i] = getLineValueX(i, MendBasis_left[0][0], MendBasis_left[1][0], k_left);
                        imageLine.Exist_Left[i] = true;
                    }

                }
                return;
            }
            //开始补线
            for (i = 0; i < road_Bottom_Right; i++)
            {
                if (!imageLine.Exist_Left[i])
                {
                    imageLine.Point_Left[i] = getLineValue(i, k_left, b_left);
                    imageLine.Exist_Left[i] = true;
                }

            }
        }
        /*imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 65;
        imageLine.Lost_Center = false;*/
    }
}
 uint8_t turnpoint_flag = 0;
/*****************************************************************
*Function: ring_out_turnPoint_filter(*)
*Description: 出环找拐点并滤除拐点上面的边界点
              使用需补充圆环状态的条件
              检测上下两行白点数
*parameter: *
*Return: *
*****************************************************************/
void ring_out_turnPoint_filter_mend(void)
{
    uint8_t i = 0, j = 0;//定义工具变量
    short flag_leftRing_Lost = 0;
    short flag_rightRing_Lost = 0;

    //恢复初始化
    ring_out_turnPoint_row = EFFECTIVE_ROW;
    ring_out_turnPoint_line = 0;
    short count;
    count = 0;
    //左圆环
    if (flag_isLeft_ring && !find_ring_Left)
    {
        find_ring_flag_Right = 0;
        find_ring_Right = 0;
        for(i = RING_OUT_TURNPOINT_START_ROW; i > RING_OUT_TURNPOINT_END_ROW; i--)
        {
            if(imageLine.Exist_Right[i] && imageLine.Exist_Right[i-1])
            {
                if(imageLine.Point_Right[i]<imageLine.Point_Right[i-1])
                {
                    ring_out_turnPoint_row = i;
                    ring_out_turnPoint_line = imageLine.Point_Right[i];
                    flag_leftRing_Lost = 1;
                    break;
                }
            }
            else if(imageLine.Exist_Right[i])
            {
                for(j = i - 1; j > RING_OUT_TURNPOINT_END_ROW; j--)
                {
                    if(imageLine.Exist_Right[j] && (i - j + 1 > 4) && imageLine.Point_Right[j] > imageLine.Point_Right[i])
                    {
                        flag_leftRing_Lost = 1;
                        ring_out_turnPoint_row = i;
                        ring_out_turnPoint_line = imageLine.Point_Right[i];
                        break;
                    }
                }
                if(flag_leftRing_Lost)
                    break;
            }
        }
        if(flag_leftRing_Lost)
        {
            turnpoint_flag = 1;
            for (i = 0; i < MT9V03X_H; i++)
            {
                if(imageLine.Exist_Left[i])
                    imageLine.Exist_Left[i]=false;
            }
            for (i = 0; i < ring_out_turnPoint_row; i++)
            {
                if(imageLine.Exist_Right[i])
                    imageLine.Exist_Right[i]=false;
            }
            imageLine.Lost_Left = true;
            short MendBasis_right[2][5];
            float k_right, b_right;

            for (i = ring_out_turnPoint_row; i < MT9V03X_H; i++)
            {
                if (imageLine.Exist_Right[i])
                {
                    MendBasis_right[0][count] = i;
                    MendBasis_right[1][count] = imageLine.Point_Right[i];
                    count++;
                }
                if (count == 5)
                    break;
            }
            if (count == 5)//有5个点即可开始补线
            {
                leastSquareMethod(MendBasis_right[0], MendBasis_right[1], 5, &k_right, &b_right);
                if(k_right<2)
                {
                    k_right = 2;
                    for (i = 0; i < ring_out_turnPoint_row; i++)
                    {
                        if (!imageLine.Exist_Right[i])
                        {
                            imageLine.Point_Right[i] = getLineValueX(i, MendBasis_right[0][0], MendBasis_right[1][0], k_right);
                            imageLine.Exist_Right[i] = true;
                        }

                    }
                    return;
                }
                //开始补线
                for (i = 0; i < ring_out_turnPoint_row; i++)
                {
                    if (!imageLine.Exist_Right[i])
                    {
                        imageLine.Point_Right[i] = getLineValue(i, k_right, b_right);
                        imageLine.Exist_Right[i] = true;
                    }

                }
            }
        }
    }
    //右圆环
    else if (flag_isRight_ring && !find_ring_Right)
    {
        find_ring_flag_Left = 0;
        find_ring_Left = 0;


        for(i = RING_OUT_TURNPOINT_START_ROW; i > RING_OUT_TURNPOINT_END_ROW; i--)
        {
            if(imageLine.Exist_Left[i] && imageLine.Exist_Left[i-1])
            {
                if(imageLine.Point_Left[i]>imageLine.Point_Left[i-1])
                {
                    flag_rightRing_Lost = 1;
                    ring_out_turnPoint_row = i;
                    ring_out_turnPoint_line = imageLine.Point_Left[i];
                    break;
                }
            }
            else if(imageLine.Exist_Left[i])
            {
                for(j = i - 1; j > RING_OUT_TURNPOINT_END_ROW; j--)
                {
                    if(imageLine.Exist_Left[j] && i - j + 1 > 4 && imageLine.Point_Left[j] < imageLine.Point_Left[i])
                    {
                        flag_rightRing_Lost = 1;
                        ring_out_turnPoint_row = i;
                        ring_out_turnPoint_line = imageLine.Point_Left[i];
                        break;
                    }
                }
                if(flag_rightRing_Lost)
                    break;
            }
        }
        if(flag_rightRing_Lost)
        {
            turnpoint_flag = 1;
            short MendBasis[2][5];
            float k, b;

            for (i = 0; i < MT9V03X_H; i++)
            {
                if(imageLine.Exist_Right[i])
                    imageLine.Exist_Right[i]=false;
            }
            for (i = 0; i < ring_out_turnPoint_row; i++)
            {
                if(imageLine.Exist_Left[i])
                    imageLine.Exist_Left[i]=false;
            }
            imageLine.Lost_Right = true;
            for (i = ring_out_turnPoint_row; i < MT9V03X_H; i++)
            {
                if (imageLine.Exist_Left[i])
                {
                    MendBasis[0][count] = i;
                    MendBasis[1][count] = imageLine.Point_Left[i];
                    count++;
                }
                if (count == 5)
                    break;
            }
            if (count == 5)//有5个点即可开始补线
            {
                leastSquareMethod(MendBasis[0], MendBasis[1], 5, &k, &b);
                if(k > -2)
                {
                    k = -2;
                    for (i = 0; i < ring_out_turnPoint_row; i++)
                    {
                        if (!imageLine.Exist_Left[i])
                        {
                            imageLine.Point_Left[i] = getLineValueX(i, MendBasis[0][0], MendBasis[1][0], k);
                            imageLine.Exist_Left[i] = true;
                        }

                    }
                    return;
                }
                //开始补线
                for (i = 0; i < ring_out_turnPoint_row; i++)
                {
                    if (!imageLine.Exist_Left[i])
                    {
                        imageLine.Point_Left[i] = getLineValue(i, k, b);
                        imageLine.Exist_Left[i] = true;
                    }

                }
            }
        }
    }

}

/*****************************************************************
*Function: ring_LostCenter_mend(*)
*Description: 出入环丢线补线
              直接给偏差
*parameter: *
*Return: *
*****************************************************************/
void ring_LostCenter_mend(void)
{
    short j = 0, count = 0, black_count = 0;
    short flag_LostCenter_Left = 0;
    short flag_LostCenter_Right = 0;

    if(ring_in_lost_left)
    {
        imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 39;
        imageLine.Lost_Center = false;
        ring_in_lost_left = 0;
    }
    else if(ring_in_lost_right)
    {
        imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 55;
        imageLine.Lost_Center = false;
        ring_in_lost_right = 0;
    }
    if (flag_isLeft_ring && !find_ring_Left)
    {
        for(j = MT9V03X_H_3; j < MT9V03X_H; j++)
        {
            if(imageLine.White_Num[j]>IS_WHITE_ROW_NUM && !(imageLine.Exist_Left[j] && imageLine.Exist_Right[j]))
                count++;
            if(imageLine.White_Num[j]<IS_BLACK_ROW_NUM)
                black_count++;
        }
        if(count > RING_LOST_COUNT && black_count > 0)
        {
            flag_LostCenter_Left = 1;
        }
        else
        {
            count = 0;
            /*for(j = 0; j < MT9V03X_H - 1; j++)
            {
                if(imageLine.Exist_Right[j] && imageLine.Exist_Right[j + 1])
                    if(imageLine.Point_Right[j + 1]-imageLine.Point_Right[j]>3)
                        count++;
                if(count > 20)
                {
                    flag_LostCenter_Left = 1;
                    break;
                }
            }*/
        }
        if(imageLine.Lost_Right || flag_LostCenter_Left || imageLine.Lost_Center)
        {
            imageLine.Lost_Right = true;
            imageLine.Exist_Center[AIM_LINE_SET] = true;
            imageLine.Point_Center[AIM_LINE_SET] = 35;
            imageLine.Lost_Center = false;
        }
    }
    else if (flag_isRight_ring && !find_ring_Right)
    {
        count = 0;
        for(j = MT9V03X_H_3; j < MT9V03X_H; j++)
        {
            if(imageLine.White_Num[j]>IS_WHITE_ROW_NUM && !(imageLine.Exist_Left[j] && imageLine.Exist_Right[j]))
                count++;
            if(imageLine.White_Num[j]<IS_BLACK_ROW_NUM)
                black_count++;
        }
        if(count > RING_LOST_COUNT && black_count > 0)
        {
            flag_LostCenter_Right = 1;
        }
        else
        {
            count = 0;
            /*for(j = 0; j < MT9V03X_H - 1; j++)
            {
                if(imageLine.Exist_Left[j] && imageLine.Exist_Left[j + 1])
                    if(imageLine.Point_Left[j]-imageLine.Point_Left[j + 1]>3)
                        count++;
                if(count > 20)
                {
                    flag_LostCenter_Right = 1;
                    break;
                }
            }*/
        }
        if(imageLine.Lost_Left || flag_LostCenter_Right || imageLine.Lost_Center)
        {
            imageLine.Lost_Right = true;
            imageLine.Exist_Center[AIM_LINE_SET] = true;
            imageLine.Point_Center[AIM_LINE_SET] = 56;
            imageLine.Lost_Center = false;
        }
    }
}

/*****************************************************************
*Function: ring_Check(uint8_t way)
*Description: 清除检测圆环标志位
*parameter: way：0左环 1右环
*Return: *
*****************************************************************/
void ring_Check(uint8_t way)
{
    if(way == 0)
    {
        if(!isRightLineStraight || imageLine.Lost_Right )
        {
            find_ring_Left=0;
            flag_isLeft_ring=0;
            find_ring_Right=0;
            flag_isRight_ring=0;
            find_ring_flag_Left=0;
            find_ring_flag_Right=0;
            isLeft = 0;
            isRight = 0;
        }
    }
    else
    {
        if(!isLeftLineStraight || imageLine.Lost_Left)
        {
            find_ring_Left=0;
            flag_isLeft_ring=0;
            find_ring_Right=0;
            flag_isRight_ring=0;
            find_ring_flag_Left=0;
            find_ring_flag_Right=0;
            isLeft = 0;
            isRight = 0;
        }
    }
}

/*****************************************************************
*Function: link_Mend(*)
*Description:  将叉字路口，尤其是环岛分叉处封闭，防止串道
*parameter: *
*Return: *
*****************************************************************/
void link_Mend(void)
{
    if(garage_in || turnpoint_flag)
    {
        turnpoint_flag = 0;
        return;
    }
   uint8_t i,j,k;
   //if(isLeft || isRight)
   for(i = 3; i < MT9V03X_H - 1; i++)
   {
       if((flag_fullcross_left || flag_fullcross_right) && i < MT9V03X_H_4)
       {
           if(imageLine.Exist_Left[i] && imageLine.Point_Left[i] < MT9V03X_W_3)
               imageLine.Exist_Left[i] = false;
           if(imageLine.Exist_Right[i] && imageLine.Point_Right[i] < MT9V03X_W_2_3)
              imageLine.Exist_Right[i] = false;
       }
       if(!isStraightLeft(i))
       {
           imageLine.Exist_Left[i+1] = false;
           imageLine.Exist_Left[i] = false;
       }
       if(!isStraightRight(i))
       {
           imageLine.Exist_Right[i+1] = false;
           imageLine.Exist_Right[i] = false;
       }
       if(imageLine.Exist_Left[i+1] && imageLine.Exist_Left[i] && imageLine.Point_Left[i+1]>imageLine.Point_Left[i])
       {
           imageLine.Exist_Left[i+1] = false;
          imageLine.Exist_Left[i] = false;
       }
       if(imageLine.Exist_Right[i+1] && imageLine.Exist_Right[i] && imageLine.Point_Right[i+1]<imageLine.Point_Right[i])
       {
           imageLine.Exist_Right[i+1] = false;
           imageLine.Exist_Right[i] = false;
       }
       if(isLeft || isRight)
       {
           if(imageLine.Exist_Left[i+1] && imageLine.Exist_Left[i] && imageLine.Exist_Left[i - 1] &&\
               imageLine.Point_Left[i] == 1 + imageLine.Point_Left[i+1] &&\
               imageLine.Point_Left[i-1] == 1 + imageLine.Point_Left[i])
           {
               imageLine.Exist_Left[i+1] = false;
               imageLine.Exist_Left[i] = false;
           }
           if(imageLine.Exist_Right[i+1] && imageLine.Exist_Right[i] && imageLine.Exist_Right[i - 1] &&\
                              imageLine.Point_Right[i+1] == 1 + imageLine.Point_Right[i]\
                              && imageLine.Point_Right[i] == 1 + imageLine.Point_Right[i - 1])
           {
               imageLine.Exist_Right[i+1] = false;
               imageLine.Exist_Right[i] = false;
           }
       }
   }
   singlePoint_Filter();
   for(i = 6; i < MT9V03X_H_5_6; i++)
   {
       if(imageLine.Exist_Left[i - 1] && imageLine.Exist_Left[i] && !imageLine.Exist_Left[i + 1] )
       {

           for(j = i + 1; j < MT9V03X_H; j++)
           {
               if(imageLine.Exist_Left[j])
               {
                   short x1 = imageLine.Point_Left[j];
                   uint8_t y1 = j;
                   short x2 = imageLine.Point_Left[i];
                   uint8_t y2 = i;
                   for (k = y2; k <= y1; k++)
                   {
                       imageLine.Exist_Left[k] = true;
                       imageLine.Point_Left[k] = (short)((x2 - x1) * (k - y1) / (y2 - y1) + x1);
                   }
                   break;
               }
           }
           if(j == MT9V03X_H)
           {

               short x1 = BOTTOM_LEFT;
               uint8_t y1 = MT9V03X_H - 1;
               short x2 = imageLine.Point_Left[i];
               uint8_t y2 = i;
               for (k = y2; k <= y1; k++)
               {
                   imageLine.Exist_Left[k] = true;
                   imageLine.Point_Left[k] = (short)((x2 - x1) * (k - y1) / (y2 - y1) + x1);
               }
               /*short MendBasis[2][5];
                float k, b;
                uint8_t count = 0;
                short first_point=0;
                for (i = MT9V03X_H - 1; i > 0; i--)
                {
                    if (imageLine.Exist_Left[i])
                    {
                        if(first_point != 0)
                            first_point = i;
                        MendBasis[0][count] = i;
                        MendBasis[1][count] = imageLine.Point_Left[i];
                        count++;
                    }
                    if (count == 5)
                        break;
                }
                if (count == 5)//有5个点即可开始补线
                {
                    leastSquareMethod(MendBasis[0], MendBasis[1], 5, &k, &b);
                    //开始补线
                    for (i = MT9V03X_H - 1; i > first_point; i--)
                    {
                        if (!imageLine.Exist_Left[i])
                        {
                            imageLine.Point_Left[i] = getLineValue(i, k, b);
                            imageLine.Exist_Left[i] = true;
                        }

                    }
                }*/
          }
       }
       if(imageLine.Exist_Right[i] && imageLine.Exist_Right[i- 1] && !imageLine.Exist_Right[i+1])
       {
          for(j = i + 1; j < MT9V03X_H; j++)
          {
              if(imageLine.Exist_Right[j])
              {
                  short x1 = imageLine.Point_Right[j];
                  uint8_t y1 = j;
                  short x2 = imageLine.Point_Right[i];
                  uint8_t y2 = i;
                  for (k = y2; k <= y1; k++)
                  {
                      imageLine.Exist_Right[k] = true;
                      imageLine.Point_Right[k] = (short)((x2 - x1) * (k - y1) / (y2 - y1) + x1);
                  }
                  break;
              }
          }

          if(j == MT9V03X_H)
          {

             short x1 = BOTTOM_RIGHT;
             uint8_t y1 = MT9V03X_H - 1;
             short x2 = imageLine.Point_Right[i];
             uint8_t y2 = i;
             for (k = y2; k <= y1; k++)
             {
                 imageLine.Exist_Right[k] = true;
                 imageLine.Point_Right[k] = (short)((x2 - x1) * (k - y1) / (y2 - y1) + x1);
             }
              /*short MendBasis[2][5];
             float k, b;
             uint8_t count = 0;
             short first_point=0;
             for (i = MT9V03X_H - 1; i > 0; i--)
             {
                 if (imageLine.Exist_Right[i])
                 {
                     if(first_point != 0)
                         first_point = i;
                     MendBasis[0][count] = i;
                     MendBasis[1][count] = imageLine.Point_Right[i];
                     count++;
                 }
                 if (count == 5)
                     break;
             }
             if (count == 5)//有5个点即可开始补线
             {
                 leastSquareMethod(MendBasis[0], MendBasis[1], 5, &k, &b);
                 //开始补线
                 for (i = MT9V03X_H - 1; i > first_point; i--)
                 {
                     if (!imageLine.Exist_Right[i])
                     {
                         imageLine.Point_Right[i] = getLineValue(i, k, b);
                         imageLine.Exist_Right[i] = true;
                     }

                 }
             }*/
          }
      }
   }
   left_right_Limit();
   lostLine_Filter();
}



uint8_t crossLeft_flag = 0;
uint8_t crossRight_flag = 0;
uint8_t flag_crossLeft_find = 0;
uint8_t flag_crossRight_find = 0;
uint8_t find_cross_Lost = 0;


/*****************************************************************
*Function: crossFilter_tsy(void)
*Description: 全十字检测
*parameter:
*Return:
*****************************************************************/
void crossFilter_tsy(void)
{
    uint8_t i, flag_left = 0, flag_right = 0, turn_row_left = 0, turn_row_right = 0;

    for(i = MT9V03X_H-3; i > MT9V03X_H_6; i--)
    {
        if(flag_right == 0 && imageLine.Exist_Right[i+1] && !imageLine.Exist_Right[i])
        {
            if(i < MT9V03X_H_3)
                flag_right = 5;
            flag_right++;
            turn_row_right = i;
            //printf("a%d\r\n",turn_row);
        }
        if(flag_left == 0 && imageLine.Exist_Left[i+1] && !imageLine.Exist_Left[i])
        {
            if(i < MT9V03X_H_3)
                flag_left = 5;
            flag_left++;
            turn_row_left = i;
        }
        if(flag_left == 1 && isWhite(0, i) && isWhite(0, i - 1) && isWhite(0, i - 2) && turn_row_left-i < 6)
        {
            turn_row_left = i;
            flag_left++;
            //printf("b%d\r\n",turn_row);
        }
        if(flag_right == 1 && isWhite(MT9V03X_H - 1, i) && isWhite(MT9V03X_H - 1, i - 1) &&\
                isWhite(MT9V03X_H - 1, i - 2) && turn_row_right-i < 6)
        {
            turn_row_right = i;
            flag_right++;
        }
        if(((flag_right == 2 && turn_row_right-i > 8)) \
                && imageLine.Exist_Right[i] && imageLine.Exist_Left[i] )
        {
            //printf("c%d\r\n",i);
            flag_fullcross_right = 1;
            crossLeft_flag = 0;

        }
        if(flag_left == 2&& turn_row_left-i > 8 && imageLine.Exist_Right[i] && imageLine.Exist_Left[i])
        {
            flag_fullcross_left = 1;
            crossRight_flag = 0;
        }

    }

}
/*****************************************************************
*Function: crossPreDetect(uint8_t way)
*Description: 十字检测第一步
*        -----
*       /     \
*       |  口  |       ------
*       \      |       ↑ 摄像头显示范围
*        ---|  |       ↓
*           |  |       ------
*        -----
*       /     \
*       |  口  |      ------
*       |     /       ↑ 摄像头显示范围
*       |  |--        ↓
*       |  |          -------
*parameter: 0：左十字 1：右十字
*Return: true/false
*****************************************************************/
bool crossPreDetect(uint8_t way)
{
    uint8_t i,flag = 0,turn_row = 0;
    if(way == 0)
    {
        for(i = MT9V03X_H-3; i > MT9V03X_H_6; i--)
        {
            if(flag == 0 && imageLine.Exist_Right[i]&&imageLine.Exist_Left[i+1] && !imageLine.Exist_Left[i])
            {
                if(i < MT9V03X_H_2)
                    return false;
                flag++;
                turn_row = i;
                //printf("a%d\r\n",turn_row);
            }
            if(flag == 1 && isWhite(0, i) && isWhite(0, i - 1) && isWhite(0, i - 2) && turn_row - i < 6)
                if(!imageLine.Exist_Left[i] && !imageLine.Exist_Left[i-1] && !imageLine.Exist_Left[i-2]\
                        && imageLine.Exist_Right[i] && imageLine.Exist_Right[i-1] && imageLine.Exist_Right[i-2])
                {
                    turn_row = i;
                    flag++;
                    //printf("b%d\r\n",turn_row);
                }
            if(flag == 2 && imageLine.Exist_Right[i] && imageLine.Exist_Left[i] && turn_row - i  > 8)
            {
                //printf("c%d\r\n",i);
                return true;

            }
        }
        return false;
    }
    else
    {
        for(i = MT9V03X_H-3; i > MT9V03X_H_6; i--)
        {
           if(flag == 0 && imageLine.Exist_Left[i]&&imageLine.Exist_Right[i+1]&& !imageLine.Exist_Right[i])
           {
               if(i < MT9V03X_H_2)
                   return false;
               flag++;
               turn_row = i;
               //printf("a%d\r\n",turn_row);
           }
           if(flag == 1 && isWhite(MT9V03X_W - 2, i) && isWhite(MT9V03X_W - 2, i - 1) && isWhite(MT9V03X_W - 2, i - 2) && turn_row - i  < 6)
               if(!imageLine.Exist_Right[i] && !imageLine.Exist_Right[i-1] && !imageLine.Exist_Right[i-2]\
                       && imageLine.Exist_Left[i] && imageLine.Exist_Left[i-1] && imageLine.Exist_Left[i-2])
               {
                   turn_row = i;
                   flag++;
                   //printf("b%d\r\n",turn_row);
               }
           if(flag == 2 && imageLine.Exist_Right[i] && imageLine.Exist_Left[i] && turn_row - i  > 8)
           {
               //printf("c%d\r\n",i);
               return true;
           }
        }
        return false;
    }
}

/*****************************************************************
*Function: crossDetect_tsy(*)
*Description: 十字检测打包
*parameter:
*Return:
*****************************************************************/
void crossDetect_tsy(void)
{
    if(!flag_crossLeft_find && !flag_crossRight_find)
    {
            //printf("%d %d %d %d \r\n",road_Width_L(i,0),road_Width_L(i-1,0),road_Width_R(i,0),road_Width_R(i-1,0));
            if(crossPreDetect(0))
                crossLeft_flag=1;
            if(crossPreDetect(1))
                crossRight_flag=1;
    }
    if((crossLeft_flag == 1 || crossRight_flag == 1))
    {
        if(crossLeft_flag==1)
            crossDetect_Left_tsy();
        else if(crossRight_flag==1)
            crossDetect_Right_tsy();
    }
    if(crossLeft_flag==2 || crossRight_flag == 2)
    {
        if(!isLeftLineStraight && !isRightLineStraight)
        {
            if(crossLeft_flag==2)
            {
                crossLeft_flag=0;
                flag_crossLeft_find++;
            }
            if(crossRight_flag==2)
            {
                crossRight_flag=0;
                flag_crossRight_find++;
            }
        }


    }
}

/*****************************************************************
*Function: crossDetect_Left_tsy(*)
*Description: 第二步检测：左十字检测
*                      ------
*        -----         ↑
*       /     \         摄像头显示范围
*       |  口  |       ↓
*       \      |       ------
*        ---|  |
*           |  |
*parameter: *
*Return:*
*****************************************************************/
void crossDetect_Left_tsy(void)
{
    uint8_t i,j,isWBW_flag = 0,count = 0,line = 0;
    if(!isRightLineStraight && !imageLine.Exist_Left[MT9V03X_H-3] && imageLine.Exist_Right[MT9V03X_H-3])
        for(i = 0; i < MT9V03X_W_2; i++)
        {
            for(j = MT9V03X_H_2; j > 0; j--)
                if(isWhite(i,j) && !isWhite(i,j+1))
                {
                    isWBW_flag++;
                    line = j;
                        break;
                }
            if(isWBW_flag == 1)
            {
                for(j=line; j < MT9V03X_H-1; j++)
                    if(!isWhite(i,j) && isWhite(i,j+1) && j - line > 10)
                    {
                        isWBW_flag++;
                        if(isWBW_flag > 2)
                            break;
                    }
                if(isWBW_flag == 2)
                    count++;
            }
            if(count > CROSS_DETECT_LINE_COUNT)
            {
                crossLeft_flag++;

                //printf("3");
                break;
            }
            isWBW_flag=0;
        }
}

/*****************************************************************
*Function: crossDetect_Right_tsy(*)
*Description: 第二步十字检测：右十字检测
*        -----         ------
*       /     \        ↑ 摄像头显示范围
*       |  口  |       ↓
*       |     /        ------
*       |  ---
*       |  |
*parameter: *
*Return:*
*****************************************************************/
void crossDetect_Right_tsy(void)
{
    uint8_t i,j,isWBW_flag = 0,count = 0,line = 0;
    if(!isLeftLineStraight && !imageLine.Exist_Right[MT9V03X_H-3] && imageLine.Exist_Left[MT9V03X_H-3])
        for(i = MT9V03X_W_2; i < MT9V03X_W; i++)
        {
            for(j = MT9V03X_H_2; j > 0; j--)
                if(isWhite(i,j)&&!isWhite(i,j+1))
                {
                    isWBW_flag++;
                    line = j;
                    break;
                }
            if(isWBW_flag == 1)
            {
                for(j=line; j < MT9V03X_H-1; j++)
                    if(!isWhite(i,j) && isWhite(i,j+1) && j - line > 10)
                    {
                        isWBW_flag++;
                        if(isWBW_flag > 2)
                            break;
                    }
                if(isWBW_flag == 2)
                    count++;
            }

            if(count > CROSS_DETECT_LINE_COUNT)
            {
                crossRight_flag++;

                //printf("4");
                break;
            }
            isWBW_flag=0;
        }

}

/*****************************************************************
*Function: crossout_mend(*)
*Description: 出十字 补线（直接给偏差）
*parameter:
*Return:
*****************************************************************/
void crossout_mend(void)
{
    short i = 0, j = 0, count = 0, black_count = 0,exist_count=0;
    short flag_LostCenter_Left = 0;
    short flag_LostCenter_Right = 0;
    if(isLeftLineStraight && isRightLineStraight)
    {
        flag_crossLeft_find = 0;
        flag_crossRight_find = 0;

    }
    if (flag_crossLeft_find)
    {
        for(j = MT9V03X_H_2; j < MT9V03X_H; j++)
        {
            if(imageLine.White_Num[j]>IS_WHITE_ROW_NUM)
                count++;
            if(imageLine.White_Num[j]<IS_BLACK_ROW_NUM)
                black_count++;
        }
        if(count > CROSS_LOST_ROW_COUNT)
        {
            flag_LostCenter_Left = 1;
        }
        if((flag_LostCenter_Left))
        {

            find_cross_Lost=1;
            imageLine.Lost_Right = true;
            imageLine.Exist_Center[AIM_LINE_SET] = true;
            imageLine.Point_Center[AIM_LINE_SET] = 80;
            imageLine.Lost_Center = false;
        }
        if(find_cross_Lost)
        {
            for(i = 0; i < MT9V03X_H; i++)
                if(imageLine.Exist_Right[i])
                    exist_count++;
            if(exist_count > CROSS_OUT_EXIST)
            {
                find_cross_Lost = 0;
                flag_crossLeft_find = 0;
            }
            else if(crossPreDetect(0) && crossPreDetect(1))
            {
                find_cross_Lost = 0;
                flag_crossLeft_find = 0;
            }
        }
    }
    else if (flag_crossRight_find)
    {
        count = 0;
        for(j = MT9V03X_H_2; j < MT9V03X_H; j++)
        {
            if(imageLine.White_Num[j]>IS_WHITE_ROW_NUM)
                count++;
            if(imageLine.White_Num[j]<IS_BLACK_ROW_NUM)
                black_count++;
        }
        if(count > CROSS_LOST_ROW_COUNT)
        {
            flag_LostCenter_Right = 1;
        }
        if((flag_LostCenter_Right))
        {
            find_cross_Lost = 1;
            imageLine.Lost_Right = true;
            imageLine.Exist_Center[AIM_LINE_SET] = true;
            imageLine.Point_Center[AIM_LINE_SET] = 14;
            imageLine.Lost_Center = false;
        }
        if(find_cross_Lost)
        {
            for(i = 0; i < MT9V03X_H; i++)
                if(imageLine.Exist_Left[i])
                    exist_count++;
            if(exist_count > CROSS_OUT_EXIST)
            {
                find_cross_Lost = 0;
                flag_crossRight_find = 0;
            }
            else if(crossPreDetect(0) && crossPreDetect(1))
            {
                find_cross_Lost = 0;
                flag_crossRight_find = 0;
            }
        }

    }
}

/*****************************************************************
*Function: cross_Check(*)
*Description: 未检测到十字时清除标志位
*parameter:
*Return:
*****************************************************************/
void cross_Check(void)
{
    if(imageLine.Lost_Left || imageLine.Lost_Right)
    {
        crossLeft_flag = 0;
        crossRight_flag = 0;
    }
}

/*****************************************************************
*Function: image_pre_processing(*)
*Description: 图像预处理函数(打包所有预处理步骤)
*parameter: *
*Return: *
*****************************************************************/
void image_pre_processing(void)
{
    Pixle_Filter();//过滤二值化图像噪点
    left_right_Limit();//保证同一行左边界点一定在右边界点左边
}

/*****************************************************************
*Function: image_pre_processing(*)
*Description: 图像处理相关变量初始化
*parameter: *
*Return: *
*****************************************************************/
void ImageProcessInit(void)
{
    uint8_t i, j;
    //边线和中心线都不存在
    imageLine.Lost_Center = true;
    imageLine.Lost_Left = true;
    imageLine.Lost_Right = true;

    stackTopPos = -1;//栈顶指针初值

    for (i = 0; i < MT9V03X_H; i++)
    {
        //每一行的左右边界点和中心点都不存在
        imageLine.Exist_Left[i] = false;
        imageLine.Exist_Right[i] = false;
        imageLine.Exist_Center[i] = false;

        //边界点和中心点设为初始位置
        imageLine.Point_Left[i] = 1;
        imageLine.Point_Right[i] = MT9V03X_W - 1;
        imageLine.Point_Center[i] = MT9V03X_W / 2;

        //每一行的白点个数清零
        imageLine.White_Num[i] = 0;

        for (j = 0; j < MT9V03X_W; j++)
        {
            isVisited[j][i] = false;//DFS用, 所有点都还没被遍历过
        }
    }

    camERR.cam_finalCenterERR[0] = 0;
    camERR.cam_finalCenterERR[1] = 0;
    camERR.cam_finalCenterERR[2] = 0;
    camERR.K_cam = 1.0f;
}

/*****************************************************************
*Function: trackDFS(*)
*Description: DFS巡线
              深度优先搜索找出连通域内所有白点(DFS不后退)
*parameter: *
*Return: *
*****************************************************************/
void trackDFS(void)
{
    uint8_t i, j;

    //选择图片下方中点作为起始点
    if (isWhite(MT9V03X_W / 2, MT9V03X_H - 2))//若下方中点就是白点
    {
        stackTopPos++;
        roadPointStackX[stackTopPos] = MT9V03X_W / 2;
        roadPointStackY[stackTopPos] = MT9V03X_H - 2;
        isVisited[MT9V03X_W / 2][MT9V03X_H - 2] = true;
    }
    else
    {
        for (i = MT9V03X_W - 3; i >= 2; i--)//从右往左搜索
        {
            if (isWhite(i - 2, MT9V03X_H - 2) && isWhite(i - 1, MT9V03X_H - 2) && isWhite(i, MT9V03X_H - 2)
                && isWhite(i + 1, MT9V03X_H - 2) && isWhite(i + 2, MT9V03X_H - 2)//连续5个白点
                )
            {
                //搜索到就入栈
                stackTopPos++;//stackTopPos非零就表示栈非空
                roadPointStackX[stackTopPos] = i;
                roadPointStackY[stackTopPos] = MT9V03X_H - 2;
                isVisited[i][MT9V03X_H - 2] = true;
                break;
            }
        }
    }

    while (stackTopPos >= 0)
    {
        //出栈
        i = roadPointStackX[stackTopPos];
        j = roadPointStackY[stackTopPos];
        stackTopPos--;

        //处理出界，直接continue
        if ((j < EFFECTIVE_ROW) || (j > MT9V03X_H - 2) || (i < 1) || (i > MT9V03X_W - 1))
        {
            continue;
        }

        /*************以下操作原则是：遇白点入栈，遇黑点初步判断为边界点(后续还需要修正滤波补线等操作)**************/
        //堆栈存储：...左右(上)左右(上)...
        //一般情况下，上的白点会先出栈(体现为先一直往图像上方搜索)，然后再出左右
        //向左搜索
        if (!isVisited[i - 1][j])
        {
            if (isWhite(i - 1, j))
            {
                //白点入栈
                stackTopPos++;
                roadPointStackX[stackTopPos] = i - 1;
                roadPointStackY[stackTopPos] = j;
                isVisited[i - 1][j] = true;
            }
            else
            {
                //黑点初步判断为边界点
                if (isLeftPoint(i, j))
                {
                    imageLine.Point_Left[j] = i;//左线轨迹
                    imageLine.Exist_Left[j] = true;
                }
            }
        }

        //向右搜索
        if (!isVisited[i + 1][j])
        {
            if (isWhite(i + 1, j))
            {
                stackTopPos++;
                roadPointStackX[stackTopPos] = i + 1;
                roadPointStackY[stackTopPos] = j;
                isVisited[i + 1][j] = true;
            }
            else//可能找到右边界
            {
                if (isRightPoint(i, j))
                {
                    imageLine.Point_Right[j] = i;//右线轨迹
                    imageLine.Exist_Right[j] = true;
                }
            }
        }

        //向上搜索(向上不判断边界点)
        if (!isVisited[i][j - 1])
        {
            if (isWhite(i, j - 1))
            {
                stackTopPos++;
                roadPointStackX[stackTopPos] = i;
                roadPointStackY[stackTopPos] = j - 1;
                isVisited[i][j - 1] = true;
            }
        }
    }
}

/*****************************************************************
*Function: lineChangeLimit(*)
*Description: 边界点不突变
*parameter: *
*Return: *
*****************************************************************/
void lineChangeLimit(void)
{
    short i, j;
    float leftK = 0;
    float rightK = 0;

    //左边界相邻两有效点斜率检测
    for (i = MT9V03X_H - 2; i > 0; i--)//从下往上
    {
        if (imageLine.Exist_Left[i])//先找到第一个有效点
        {
            for (j = i + 1; j < MT9V03X_H; j++)//再向下找临近有效点
            {
                if (imageLine.Exist_Left[j])
                {
                    leftK = getLineK(i, imageLine.Point_Left[i], j, imageLine.Point_Left[j]);

                    if (ABS(leftK) > K_MAX_THRESHOLD)
                    {
                        imageLine.Exist_Left[i] = false;
                        //imageLine.Exist_Left[j] = false;
                    }
                    break;//只要找到一个临近有效点，检测后就break到下一个i
                }
                else
                    continue;
            }
        }
        else
            continue;
    }

    //右边界相邻两有效点斜率检测
    for (i = MT9V03X_H - 2; i > 0; i--)//从下往上
    {
        if (imageLine.Exist_Right[i])//先找到第一个有效点
        {
            for (j = i + 1; j < MT9V03X_H; j++)//再向下找临近有效点
            {
                if (imageLine.Exist_Right[j])
                {
                    rightK = getLineK(i, imageLine.Point_Right[i], j, imageLine.Point_Right[j]);

                    if (ABS(rightK) > K_MAX_THRESHOLD)
                    {
                        imageLine.Exist_Right[i] = false;
                        //imageLine.Exist_Right[j] = false;
                    }
                    break;//只要找到一个临近有效点，检测后就break到下一个i
                }
                else
                    continue;
            }
        }
        else
            continue;
    }
}

/*****************************************************************
*Function: doFilter(*)
*Description: 所有filter函数打包
*parameter: *
*Return: *
*****************************************************************/
void doFilter(void)
{
    lostLine_Filter();//无效行过多滤去(丢边线判断)
    position_Filter();//位置不对滤去

    //slope_Filter();//斜率不对滤去

    lostLine_Filter();
    if(flag_isLeft_ring || flag_isRight_ring)
    singlePoint_Filter();//测试：单独点滤去

}

/*****************************************************************
*Function: lostLine_Filter(*)
*Description: 左右边线无效行过多则判断丢边线
*parameter: *
*Return: *
*****************************************************************/
void lostLine_Filter(void)
{
    //对于左边界线的判断--------------------
    uint8_t count = 0;
    uint8_t i = 0;

    for (i = 0; i < MT9V03X_H; i++)//从上到下搜索
    {
        if (imageLine.Exist_Left[i] == true)
            count++;
    }

    if (count < VALID_LINE_THRESHOLE)//如果无效行超过阈值认为该边界线丢失
    {
        imageLine.Lost_Left = true;
        for (i = 0; i < MT9V03X_H; i++)//从上到下搜索
        {
            imageLine.Exist_Left[i] = false;
        }
    }
    else
        imageLine.Lost_Left = false;

    //对于右边界线的判断--------------------
    count = 0;
    for (i = 0; i < MT9V03X_H; i++)
    {
        if (imageLine.Exist_Right[i] == true)
            count++;
    }

    if (count < VALID_LINE_THRESHOLE)//如果无效行超过阈值认为该边界线丢失
    {
        imageLine.Lost_Right = true;
        for (i = 0; i < MT9V03X_H; i++)//从上到下搜索
        {
            imageLine.Exist_Right[i] = false;
        }
    }
    else
        imageLine.Lost_Right = false;
}

/*****************************************************************
*Function: position_Filter(*)
*Description: 把位置不对的边线滤去
              如在右边的左边线×. 过滤掉越过中间的边界点
              但是要注意，如果一边丢线，另一边可能会跑到对面去的！
*parameter: *
*Return: *
*****************************************************************/
void position_Filter(void)
{
    uint8_t i;

    //仅当左右都不丢线的时候才滤
    if (!imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        for (i = 0; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Left[i] && (imageLine.Point_Left[i] > MT9V03X_W / 2))
                imageLine.Exist_Left[i] = false;
            if (imageLine.Exist_Right[i] && (imageLine.Point_Right[i] < MT9V03X_W / 2))
                imageLine.Exist_Right[i] = false;
        }
    }
}

/*****************************************************************
*Function: slope_Filter(*)
*Description: 滤除斜率不对的边界点
              这个函数要放在十字串道后面，以免找不到十字的拐点
*parameter: *
*Return: *
*****************************************************************/
void slope_Filter(void)
{
    short i, j;
    float leftK = 0;
    float rightK = 0;

    //左边界相邻两有效点斜率检测
    for (i = MT9V03X_H - 2; i > 0; i--)//从下往上
    {
        if (imageLine.Exist_Left[i])//先找到第一个有效点
        {
            for (j = i + 1; j < MT9V03X_H; j++)//再向下找临近有效点
            {
                if (imageLine.Exist_Left[j])
                {
                    leftK = getLineK(i, imageLine.Point_Left[i], j, imageLine.Point_Left[j]);
                    if (leftK > 0 || ABS(leftK) > K_MAX_THRESHOLD)
                    {
                        //imageLine.Exist_Left[i] = false;
                        imageLine.Exist_Left[j] = false;
                    }
                    break;//只要找到一个临近有效点，检测后就break到下一个i
                }
                else
                    continue;
            }
        }
        else
            continue;
    }

    //右边界相邻两有效点斜率检测
    for (i = MT9V03X_H - 2; i > 0; i--)//从下往上
    {
        if (imageLine.Exist_Right[i])//先找到第一个有效点
        {
            for (j = i + 1; j < MT9V03X_H; j++)//再向下找临近有效点
            {
                if (imageLine.Exist_Right[j])
                {
                    rightK = getLineK(i, imageLine.Point_Right[i], j, imageLine.Point_Right[j]);

                    if (rightK < 0 || ABS(rightK) > K_MAX_THRESHOLD)//左边线理想状态是一直往右的(xzl: 或许会影响到找拐点？)
                    {
                        //imageLine.Exist_Right[i] = false;
                        imageLine.Exist_Right[j] = false;
                    }
                    break;//只要找到一个临近有效点，检测后就break到下一个i
                }
                else
                    continue;
            }
        }
        else
            continue;
    }
}

/*****************************************************************
*Function: singlePoint_Filter(*)
*Description: 单独存在的边界点滤除
*parameter: *
*Return: *
*****************************************************************/
void singlePoint_Filter(void)
{
    uint8_t i;
    for (i = EFFECTIVE_ROW; i < MT9V03X_H - 1; i++)
    {
        if (!imageLine.Exist_Left[i - 1] && imageLine.Exist_Left[i] && !imageLine.Exist_Left[i + 1])
        {
            imageLine.Exist_Left[i] = false;
        }
        if (!imageLine.Exist_Right[i - 1] && imageLine.Exist_Right[i] && !imageLine.Exist_Right[i + 1])
        {
            imageLine.Exist_Right[i] = false;
        }
    }

}

/*****************************************************************
*Function: centerChangeLimit(*)
*Description: 中心点不突变
*parameter: *
*Return: *
*****************************************************************/
void centerChangeLimit(void)
{
    short i, j;

    for (i = MT9V03X_H - 2; i > EFFECTIVE_ROW; i--)//从下往上
    {
        if (imageLine.Exist_Center[i])//找到一个中心点
        {
            for (j = i + 1; j < MT9V03X_H; j++)//再向下找临近中心点
            {
                if (imageLine.Exist_Center[j])
                {
                    if (j - i < 2)
                    {
                        if ((imageLine.Point_Center[i] - imageLine.Point_Center[j]) / (i - j) > 10)
                            imageLine.Point_Center[i] = imageLine.Point_Center[j] + 20 * (MT9V03X_W - 2 - i);
                        else if ((imageLine.Point_Center[i] - imageLine.Point_Center[j]) / (i - j) < -10)
                            imageLine.Point_Center[i] = imageLine.Point_Center[j] - 20 * (MT9V03X_W - 2 - i);
                    }
                }
            }
        }
    }
}

/*****************************************************************
*Function: road_Width_R(uint8_t row,uint8_t way)
*Description: 检测右方元素时所用的路宽检测
*parameter: row：行
*           way：0：路宽=左边线起点->右方第一个黑点
*               1：路宽=左半屏幕第一个白点->右方第一个黑点
*               2：路宽=全屏左方第一个白点->右方第一个黑点
*Return: *
*****************************************************************/
short road_Width_R(uint8_t row,uint8_t way)
{
    short i,j,whiteLeftLine = 0;
    switch(way)
    {
        case 0:
            if(imageLine.Exist_Left[row] && imageLine.Point_Left[row] < MT9V03X_W_2)
            {
                for(i = imageLine.Point_Left[row]; i < MT9V03X_W_2; i++)
                    if(isWhite(i,row))
                    {
                        whiteLeftLine=i;
                        break;
                    }
            }
            else
                return 0;
            if(i < MT9V03X_W_2)
                for(j = whiteLeftLine; j < MT9V03X_W; j++)
                {
                    if(!isWhite(j,row))
                    {
                        if(j-i)
                        break;
                    }
                }
            else
                return 0;
            return (j-i);
        case 1:
            for(i = 0; i < MT9V03X_W_2; i++)
                if(isWhite(i,row))
                {
                    whiteLeftLine=i;
                    break;
                }
            if(i < MT9V03X_W_2)
                for(j = whiteLeftLine; j < MT9V03X_W; j++)
                {
                    if(!isWhite(j,row))
                    {
                        if(j-i)
                        break;
                    }
                }
            else
                return 0;
            return (j-i);
        case 2:
            for(i = 0; i < MT9V03X_W - 1; i++)
                if(isWhite(i,row))
                {
                    whiteLeftLine=i;
                    break;
                }
            for(j = whiteLeftLine; j < MT9V03X_W; j++)
            {
                if(!isWhite(j,row))
                {
                    break;
                }
            }
            return (j-i);
    }
    return 0;
}

uint8_t road_Width_Last = 47;
/*****************************************************************
*Function: road_Width_L(uint8_t row,uint8_t way)
*Description: 检测左方元素时所用的路宽检测
*parameter: row：行
*           way：0：路宽=右边线起点->左方第一个黑点
*               1：路宽=右半屏幕第一个白点->左方第一个黑点
*               2：路宽=全屏右方第一个白点->左方第一个黑点
*Return: *
*****************************************************************/
short road_Width_L(uint8_t row, uint8_t way)
{
    short i,j,whiteRightLine = 0;
    switch(way)
    {
        case 0:

            if(imageLine.Exist_Right[row] && imageLine.Point_Right[row] > MT9V03X_W_2)
            {
                for(i = imageLine.Point_Right[row]; i > MT9V03X_W_2; i--)
                    if(isWhite(i,row))
                    {
                        whiteRightLine=i;
                        break;
                    }
            }
            else
                return 0;
            if(i > MT9V03X_W_2)
                for(j = whiteRightLine; j > 0; j--)
                {
                    if(!isWhite(j,row))
                    {
                        break;
                    }
                }
            else
                return 0;
            return (i-j);
        case 1:
            for(i = MT9V03X_W - 1; i > MT9V03X_W_2; i--)
                if(isWhite(i,row))
                {
                    whiteRightLine=i;
                    break;
                }
            if(i > MT9V03X_W_2)
                for(j = whiteRightLine; j > 0; j--)
                {
                    if(!isWhite(j,row))
                    {
                        break;
                    }
                }
            else
                return 0;
            return (i-j);
        case 2:
            whiteRightLine = 1;
            for(i = MT9V03X_W - 1; i > 20; i--)
                if(isWhite(i,row))
                {
                    whiteRightLine=i;
                    break;
                }
            if(i > 20){
            for(j = whiteRightLine; j > 0; j--)
            {
                if(!isWhite(j,row))
                {
                    break;
                }
            }
            return (i-j);}
    }
    return 0;
}

/*****************************************************************
*Function: isStraightLeft(uint8_t row)
*Description: 检测相邻两左边线偏差是否小于2
*parameter: row：行
*Return: true/false
*****************************************************************/
bool isStraightLeft(uint8_t row)
{
    if(imageLine.Exist_Left[row] && imageLine.Exist_Left[row + 1])
    {
        if(!flag_isRight_ring)
        {
            if(ABS(imageLine.Point_Left[row]-imageLine.Point_Left[row + 1])>1)
                return false;
        }
        else
        {
            if(ABS(imageLine.Point_Left[row]-imageLine.Point_Left[row + 1])>3)
                return false;
        }
    }
    return true;
}

/*****************************************************************
*Function: isStraightRight(uint8_t row)
*Description: 检测相邻两右边线偏差是否小于2
*parameter: row：行
*Return: true/false
*****************************************************************/
bool isStraightRight(uint8_t row)
{
    if(imageLine.Exist_Right[row] && imageLine.Exist_Right[row + 1])
    {
        if(!flag_isLeft_ring)
        {
            if(ABS(imageLine.Point_Right[row+1]-imageLine.Point_Right[row])>1)
                return false;
        }
        else
        {
            if(ABS(imageLine.Point_Right[row+1]-imageLine.Point_Right[row])>3)
                return false;
        }
    }
    return true;
}

void lost_mend(void)
{

    for (int i = 50; i > 10; i--)
    {
        if (!imageLine.Exist_Left[i+1] && imageLine.Exist_Left[i])
        {
            uint8_t tempPointer1;
            uint8_t tempPointer_Val1;

            tempPointer1 = i;//最上最右的左边界点
            tempPointer_Val1 = imageLine.Point_Left[i];

            while (!imageLine.Exist_Left[tempPointer1 + 1])
            {
                //右上
                if (isEdgePoint(tempPointer_Val1 - 1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    tempPointer_Val1 = tempPointer_Val1 - 1;
                    imageLine.Exist_Left[tempPointer1] = true;
                    imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
                }
                //上
                else if (isEdgePoint(tempPointer_Val1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    //tempPointer_Val1 = tempPointer_Val1;
                    imageLine.Exist_Left[tempPointer1] = true;
                    imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
                }
                //右
                else if (isEdgePoint(tempPointer_Val1 - 1, tempPointer1))
                {
                    //tempPointer1 = tempPointer1;
                    tempPointer_Val1 = tempPointer_Val1 - 1;
                    imageLine.Exist_Left[tempPointer1] = true;
                    imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
                }
                else
                    break;
            }
        }

        if (!imageLine.Exist_Right[i+1] && imageLine.Exist_Right[i])
        {
            uint8_t tempPointer1;
            uint8_t tempPointer_Val1;

            tempPointer1 = i;//最上最右的左边界点
            tempPointer_Val1 = imageLine.Point_Right[i];

            while (!imageLine.Exist_Right[tempPointer1 + 1])
            {
                //右上
                if (isEdgePoint(tempPointer_Val1 + 1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    tempPointer_Val1 = tempPointer_Val1 + 1;
                    imageLine.Exist_Right[tempPointer1] = true;
                    imageLine.Point_Right[tempPointer1] = tempPointer_Val1;
                }
                //上
                else if (isEdgePoint(tempPointer_Val1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    //tempPointer_Val1 = tempPointer_Val1;
                    imageLine.Exist_Right[tempPointer1] = true;
                    imageLine.Point_Right[tempPointer1] = tempPointer_Val1;
                }
                //右
                else if (isEdgePoint(tempPointer_Val1 + 1, tempPointer1))
                {
                    //tempPointer1 = tempPointer1;
                    tempPointer_Val1 = tempPointer_Val1 + 1;
                    imageLine.Exist_Right[tempPointer1] = true;
                    imageLine.Point_Right[tempPointer1] = tempPointer_Val1;
                }
                else
                    break;
            }
        }
    }
}
//内部变量

//内部函数
static void StraightLineJudge(void);//九宫格补边界点
static void trackMend_startPart(void);//起始段补线
static void trackMend_endPart(void);//结束段补线
static void trackMend_HalfWidth(void);//丢线半宽补线
static void repairRemainLine(void);//最小二乘法修复未知的中线
static void amplitudeLIMIT(uint8_t i, uint8_t amp);//对得出的中线进行赋值限幅
static void limitCenter(void);//对得出的中线进行突变限幅

/*****************************************************************
*Function: doMend(*)
*Description: 所有补线函数打包
*parameter: *
*Return: *
*****************************************************************/
void doMend(void)
{

        StraightLineJudge();//为后面的后补线做准备
        if(isLeftLineStraight)
            LED_Ctrl(LED2,ON);
        else
            LED_Ctrl(LED2,OFF);
        if(isRightLineStraight)
            LED_Ctrl(LED3,ON);
        else
            LED_Ctrl(LED3,OFF);
        if(!garage_in)
        {
            if((crossLeft_flag && !flag_crossLeft_find) || (crossRight_flag && !flag_crossRight_find))
                cross_Check();
            ring_detect();
            ring_out_detect();
            if(!flag_isRight_ring && !flag_isLeft_ring && !is_zebra)
                crossDetect_tsy();
            else
            {
                crossLeft_flag = 0;
                crossRight_flag = 0;
                flag_crossLeft_find = 0;
                flag_crossRight_find = 0;
            }

            if(isLeft)
                ring_Check(0);
            else if(isRight)
                ring_Check(1);


            if((flag_isLeft_ring && !find_ring_Left) || (flag_isRight_ring && !find_ring_Right))
                ring_out_turnPoint_filter_mend();
            if(find_ring_Left ||find_ring_Right)
                ring_in_Mend();
            crossFilter_tsy();
            if(isLeft || isRight ||(flag_isLeft_ring && !find_ring_Left) || (flag_isRight_ring && !find_ring_Right)
                    || crossLeft_flag || crossRight_flag || flag_fullcross_right || flag_fullcross_left)
            {
                flag_fullcross_left = 0;
                flag_fullcross_right = 0;
                flag_forkRoad_find = 0;
                link_Mend();
            }

        }
        if(!isLeft && !isRight && !flag_isRight_ring && !flag_isLeft_ring)
            trackMend_startPart();//补前端(距离车)


        if ((!isLeftLineStraight && !imageLine.Lost_Left && imageLine.Lost_Right)
            || (!isRightLineStraight && imageLine.Lost_Left && !imageLine.Lost_Right)
            || (!isLeftLineStraight && !isRightLineStraight && !imageLine.Lost_Left && !imageLine.Lost_Right)
            || !isLeft || !isRight)
            trackMend_endPart();//补末端(距离车)

        if (flag_forkRoad_find == 1 && !isLeftLineStraight && !isRightLineStraight && !flag_isRight_ring && !flag_isLeft_ring)
            forkRoad_mend(isForkRoadTurnLeft);//////////////////////////////////////////////三岔滤点与补线！！！
        else
            flag_forkRoad_find = 0;



        //track_boundary_detect();

        trackMend_HalfWidth();//丢边线半宽补，不丢线直接计算

        ring_LostCenter_mend();//出环丢线补线
        crossout_mend();
        if(garage_in)
            garage_in_mend();

    //centerChangeLimit();//中心点不突变
}

/*****************************************************************
*Function: track_boundary_detect(*)
*Description: 单边丢线时判断赛道边界的斜率
*parameter: *
*Return: *
*****************************************************************/
void track_boundary_detect(void)
{
    short i = 0, j = 0;
    short k_count = 0;

    //丢左边线，则判断右边线的斜率
    if (imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        k_count = 0;
        for (i = EFFECTIVE_ROW; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Right[i] && (imageLine.Point_Right[i] > 0 && imageLine.Point_Right[i] < 94))//只取在图片内部的边界点进行计算
            {
                if (imageLine.Exist_Right[i + 7] && imageLine.Exist_Right[i + 7 - 3])//图片内起始边界点向下7行进行计算
                {
                    track_boundary_k_right[k_count] = ((float)imageLine.Point_Right[i + 7] - (float)imageLine.Point_Right[i + 7 - 3]) / 3;
                    k_count++;

                    if (k_count >= 3)
                        break;
                }
            }
        }
        if (k_count < 3)//没找够三个斜率
        {
            flag_more_SingleLineLeanK = 0;
            return;
        }
    }

    //丢右边线，则判断左边线的斜率
    else if (!imageLine.Lost_Left && imageLine.Lost_Right)
    {
        k_count = 0;
        for (i = EFFECTIVE_ROW; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Left[i] && (imageLine.Point_Left[i] > 0 && imageLine.Point_Left[i] < 94))//只取在图片内部的边界点进行计算
            {
                if (imageLine.Exist_Left[i + 7] && imageLine.Exist_Left[i + 7 - 3])//图片内起始边界点向下7行进行计算(弯道起始斜率也会比较大)
                {
                    track_boundary_k_left[k_count] = ((float)imageLine.Point_Left[i + 7] - (float)imageLine.Point_Left[i + 7 - 3]) / 3;
                    k_count++;

                    if (k_count >= 3)
                        break;
                }
            }
        }
        if (k_count < 3)//没找够三个斜率
        {
            flag_more_SingleLineLeanK = 0;
            return;
        }
    }

    //两边都没有丢线
    else if (!imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        flag_more_SingleLineLeanK = 0;
        return;
    }

    //前面没有return说明找到了三个斜率
    //丢左边线
    if (imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        track_boundary_k_err[0] = 0;
        track_boundary_k_err[1] = 0;

        //斜率大小检测
        for (j = 0; j < 3; j++)
        {
            if (my_abs_short(track_boundary_k_right[j]) >= 4.3)
                continue;
            else
            {
                flag_more_SingleLineLeanK = 0;
                return;
            }
        }

        //没有return说明斜率大小合适
        track_boundary_k_err[0] = my_abs_short(track_boundary_k_right[1] - track_boundary_k_right[0]);
        track_boundary_k_err[1] = my_abs_short(track_boundary_k_right[2] - track_boundary_k_right[1]);
    }
    //丢右边线
    else if (!imageLine.Lost_Left && imageLine.Lost_Right)
    {
        track_boundary_k_err[0] = 0;
        track_boundary_k_err[1] = 0;

        //斜率大小检测
        for (j = 0; j < 3; j++)
        {
            if (my_abs_short(track_boundary_k_left[j]) > 4.3)
                continue;
            else
            {
                flag_more_SingleLineLeanK = 0;
                return;
            }
        }

        //没有return说明斜率大小合适
        track_boundary_k_err[0] = my_abs_short(track_boundary_k_left[1] - track_boundary_k_left[0]);
        track_boundary_k_err[1] = my_abs_short(track_boundary_k_left[2] - track_boundary_k_left[1]);
    }
    else
    {
        flag_more_SingleLineLeanK = 0;
        return;
    }

    //前面没有return说明斜率之间的偏差也算完了
    if (my_abs_short(track_boundary_k_err[0]) <= 2 && my_abs_short(track_boundary_k_err[1]) <= 2 && !flag_forkRoad_find)
    {
        flag_more_SingleLineLeanK = 1;
        for (i = 0; i < 3; i++)
        {
            track_boundary_k_right[i] = 0;
            track_boundary_k_left[i] = 0;
        }
    }
    else
    {
        flag_more_SingleLineLeanK = 0;
        for (i = 0; i < 3; i++)
        {
            track_boundary_k_right[i] = 0;
            track_boundary_k_left[i] = 0;
        }
    }
}


/*****************************************************************
*Function: StraightLineJudge(*)
*Description: 补线准备
              用最小二乘法计算直线，与实际的边界点作比较
              当err小于设定值的时候，认为此时的边界线是直的
              如果左右边界比较直就不需要九宫格寻边了
*parameter: *
*Return: *
*****************************************************************/
void StraightLineJudge(void)
{
    float k1, b1, k2, b2;
    short JudgeBasis_left[2][MT9V03X_H];
    short JudgeBasis_right[2][MT9V03X_H];

    uint8_t count1 = 0, count2 = 0;
    float err1, err2;

    uint8_t i, j;

    bool temp_leftLine_lost = false;//丢左线（临时标志
    bool temp_rightLine_lost = false;//丢右线（临时标志

    //拟合左线---------------------
    for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)
    {
        if (imageLine.Exist_Left[i])
        {
            JudgeBasis_left[0][count1] = i;
            JudgeBasis_left[1][count1] = imageLine.Point_Left[i];
            count1++;
        }
    }

    if (count1 >= VALID_LINE_THRESHOLE)
        leastSquareMethod(JudgeBasis_left[0], JudgeBasis_left[1], count1, &k1, &b1);
    else
        imageLine.Lost_Left = true;

    //拟合右线---------------------
    for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)
    {
        if (imageLine.Exist_Right[i])
        {
            JudgeBasis_right[0][count2] = i;
            JudgeBasis_right[1][count2] = imageLine.Point_Right[i];
            count2++;
        }
    }

    if (count2 >= VALID_LINE_THRESHOLE)
        leastSquareMethod(JudgeBasis_right[0], JudgeBasis_right[1], count2, &k2, &b2);
    else
        imageLine.Lost_Right = true;

    //第一次拟合后判断丢边情况---先判断左边界丢了没有 要是丢了就重新拟合；再同样判断右边界
    //如果左右边线都没有丢，就不进行下面的二次拟合
    //*************************************************
    //1. 只丢了左边线
    if (imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        temp_leftLine_lost = true;
        //左线丢了 再找一次左线----------
        for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)//从上往下
        {
            if (imageLine.Exist_Right[i])//找到右边界点
            {
                for (j = imageLine.Exist_Right[i] - 1; j > 0; j--)//从右边界点这一行往左边找是否有左边界点
                {
                    if (isLeftPoint(j, i))
                    {
                        imageLine.Exist_Left[i] = true;
                        imageLine.Point_Left[i] = j;
                        break;//只要找到一个左边界点就不找了，认为左线没有丢
                    }
                }
            }
        }
        imageLine.Lost_Left = false;

        //第二次拟合------
        count1 = 0;
        for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Left[i])
            {
                JudgeBasis_left[0][count1] = i;
                JudgeBasis_left[1][count1] = imageLine.Point_Left[i];
                count1++;
            }
        }

        if (count1 >= VALID_LINE_THRESHOLE)
            leastSquareMethod(JudgeBasis_left[0], JudgeBasis_left[1], count1, &k1, &b1);
        else
            imageLine.Lost_Left = true;
    }
    //2. 只丢了右边线
    else if (!imageLine.Lost_Left && imageLine.Lost_Right)
    {
        temp_rightLine_lost = true;
        //右线丢了 再找一次右线----------
        for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)//从上往下
        {
            if (imageLine.Exist_Left[i])//找到左边界点
            {
                for (j = imageLine.Exist_Left[i] + 1; j < MT9V03X_W; j++)//在左边界同一行开始找右边界点
                {
                    if (isRightPoint(j, i))
                    {
                        imageLine.Exist_Right[i] = true;
                        imageLine.Point_Right[i] = j;
                        break;//只要找到一个右边界点就不找了，认为右线没有丢
                    }
                }
            }
        }
        imageLine.Lost_Right = false;

        //第二次拟合------
        count2 = 0;
        for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Right[i])
            {
                JudgeBasis_right[0][count2] = i;
                JudgeBasis_right[1][count2] = imageLine.Point_Right[i];
                count2++;
            }
        }

        if (count2 >= VALID_LINE_THRESHOLE)
            leastSquareMethod(JudgeBasis_right[0], JudgeBasis_right[1], count2, &k2, &b2);
        else
            imageLine.Lost_Right = true;
    }
    //*************************************************

    //直线（拟合误差）判断-------
    if (!imageLine.Lost_Left && count1>30)//左线没丢的情况下计算左拟合误差
    {
        err1 = getLeastSquareMethodERROR(JudgeBasis_left[0], JudgeBasis_left[1], count1, k1, b1);
        //printf("L%f\r\n",err1);
        if (err1 > 0.38)
        {
            isLeftLineStraight = false;//左线是否为直线
        }
        else
        {
            isLeftLineStraight = true;//左线是否为直线
        }
    }
    if (!imageLine.Lost_Right && count2>30)//右线没丢的情况下计算右拟合误差
    {
        err2 = getLeastSquareMethodERROR(JudgeBasis_right[0], JudgeBasis_right[1], count2, k2, b2);
        //printf("R%f\r\n",err2);

        if (err2 > 0.38)
        {
            isRightLineStraight = false;//右线是否为直线
        }
        else
        {
            isRightLineStraight = true;//左线是否为直线
        }
    }
    if (imageLine.Lost_Left && imageLine.Lost_Right)//如果左右边线都丢了
    {
        isLeftRightLineExist = false;
    }
    //丢边线之后边界点都不要了-----------------------
    if (temp_leftLine_lost)
    {
        for (i = 0; i < MT9V03X_H; i++)
        {
            imageLine.Exist_Left[i] = false;
        }
        imageLine.Lost_Left = true;
    }
    if (temp_rightLine_lost)
    {
        for (i = 0; i < MT9V03X_H; i++)
        {
            imageLine.Exist_Right[i] = false;
        }
        imageLine.Lost_Right = true;
    }

}

/*****************************************************************
*Function: trackMend_startPart(*)
*Description: 初始段补线
              车体近处有一部分边线看不见，理论给它补上
*parameter: *
*Return: *
*****************************************************************/
void trackMend_startPart(void)
{
    uint8_t leftLine_startPoint = 0;
    uint8_t rightLine_startPoint = 0;
    int8_t i;

    float k_left, b_left;
    float k_right, b_right;

    short MendBasis_left[2][5];
    short MendBasis_right[2][5];
    uint8_t count = 0;

    //------------------------补左线-------------------------------
    //找左线起始点
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (imageLine.Exist_Left[i])
        {
            leftLine_startPoint = i;
            break;
        }
    }
    //当左起始超过1/6行时(图像底部看不到边线的部分有点多啦),补线
    if (leftLine_startPoint > MT9V03X_H * 5 / 6)
    {
        for (i = leftLine_startPoint; i >= max(leftLine_startPoint - 15, 0); i--)
        {
            if (imageLine.Exist_Left[i])
            {
                MendBasis_left[0][count] = i;
                MendBasis_left[1][count] = imageLine.Point_Left[i];
                count++;
            }
            if (count == 5)
                break;
        }
        if (count == 5)//有5个点即可开始补线
        {
            leastSquareMethod(MendBasis_left[0], MendBasis_left[1], 5, &k_left, &b_left);

            //开始补线
            for (i = MT9V03X_H - 1; i >= leftLine_startPoint; i--)
            {
                if (!imageLine.Exist_Left[i])
                {
                    imageLine.Point_Left[i] = getLineValue(i, k_left, b_left);
                    imageLine.Exist_Left[i] = true;
                }

            }
        }
    }

    //------------------------补右线-------------------------------
    //找右线起始点
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (imageLine.Exist_Right[i])
        {
            rightLine_startPoint = i;
            break;
        }
    }
    //当右起始超过1/6行时(图像底部看不到边线的部分有点多啦),补线
    count = 0;
    if (rightLine_startPoint > MT9V03X_H *5/ 6)
    {
        for (i = rightLine_startPoint; i >= max(rightLine_startPoint - 15, 0); i--)
        {
            if (imageLine.Exist_Right[i])
            {
                MendBasis_right[0][count] = i;
                MendBasis_right[1][count] = imageLine.Point_Right[i];
                count++;
            }
            if (count == 5) break;
        }
        if (count == 5)//有5个点即可开始补线
        {
            leastSquareMethod(MendBasis_right[0], MendBasis_right[1], 5, &k_right, &b_right);

            //开始补线
            for (i = MT9V03X_H - 1; i >= rightLine_startPoint; i--)
            {
                if (!imageLine.Exist_Right[i])
                {
                    imageLine.Point_Right[i] = getLineValue(i, k_right, b_right);
                    imageLine.Exist_Right[i] = true;
                }
            }
        }
    }
}

/*****************************************************************
*Function: trackMend_endPart(*)
*Description: 结束段补线
              九宫格补边界线
*parameter: *
*Return: *
*****************************************************************/
void trackMend_endPart(void)
{
    bool leftIsAllRight = true; //左线是否一直向右
    bool rightIsAllLeft = true;//右线是否一直向左

    uint8_t leftTopPoint = 0;//左边界最高有效点
    uint8_t rightTopPoint = 0;//有边界最高有效点

    uint8_t count = 0;

    int8_t i, j;//永远的工具i工具j
    uint8_t tempPointer1;
    short tempPointer_Val1;
    uint8_t tempPointer2;
    short tempPointer_Val2;
    //九宫格补左线---------------------------------
    //1. 判断左线是否一直向右
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (imageLine.Exist_Left[i])
        {
            for (j = i - 1; j >= 0; j--)
            {
                if (imageLine.Exist_Left[j])
                {
                    if (imageLine.Point_Left[j] - imageLine.Point_Left[i] < 0)
                    {
                        leftIsAllRight = false;
                        break;//下一个i
                    }
                }
            }
            count++;
            leftTopPoint = i;//最上最右的左边界点
        }
    }
    //2. 左线满足一直往右，开始八点寻边
    if (leftIsAllRight && (count > 15))
    {


        tempPointer1 = leftTopPoint;//最上最右的左边界点
        tempPointer_Val1 = imageLine.Point_Left[tempPointer1];

        while (1)
        {
            //右上
            if (isEdgePoint(tempPointer_Val1 + 1, tempPointer1 - 1))
            {
                tempPointer1 = tempPointer1 - 1;
                tempPointer_Val1 = tempPointer_Val1 + 1;
                imageLine.Exist_Left[tempPointer1] = true;
                imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
            }
            //上
            else if (isEdgePoint(tempPointer_Val1, tempPointer1 - 1))
            {
                tempPointer1 = tempPointer1 - 1;
                imageLine.Exist_Left[tempPointer1] = true;
                imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
            }
            //右
            else if (isEdgePoint(tempPointer_Val1 + 1, tempPointer1))
            {
                tempPointer_Val1 = tempPointer_Val1 + 1;
                imageLine.Exist_Left[tempPointer1] = true;
                imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
            }
            else
                break;
        }
    }

    //九宫格补右线---------------------------------
    count = 0;
    //1. 判断右线是否一直向左
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (imageLine.Exist_Right[i])
        {
            for (j = i + 1; j < MT9V03X_H; j++)
            {
                if (imageLine.Exist_Right[j])
                {
                    if (imageLine.Point_Right[j] - imageLine.Point_Right[i] < 0)
                    {
                        rightIsAllLeft = false;
                        break;//下一个i
                    }
                }
            }
            count++;
            rightTopPoint = i;//最上最左的右边界点
        }
    }
    //2. 右线满足一直往左，开始八点寻边
    if (rightIsAllLeft && (count > 15))
    {


        tempPointer2 = rightTopPoint;//最上最左的右边界点
        tempPointer_Val2 = imageLine.Point_Right[tempPointer2];

        while (1)
        {
            //左上
            if (isEdgePoint(tempPointer_Val2 - 1, tempPointer2 - 1))
            {
                tempPointer2 = tempPointer2 - 1;
                tempPointer_Val2 = tempPointer_Val2 - 1;
                imageLine.Exist_Right[tempPointer2] = true;
                imageLine.Point_Right[tempPointer2] = tempPointer_Val2;
            }
            //上
            else if (isEdgePoint(tempPointer_Val2, tempPointer2 - 1))
            {
                tempPointer2 = tempPointer2 - 1;
                imageLine.Exist_Right[tempPointer2] = true;
                imageLine.Point_Right[tempPointer2] = tempPointer_Val2;
            }
            //右
            else if (isEdgePoint(tempPointer_Val2 - 1, tempPointer2))
            {
                tempPointer_Val2 = tempPointer_Val2 - 1;
                imageLine.Exist_Right[tempPointer2] = true;
                imageLine.Point_Right[tempPointer2] = tempPointer_Val2;
            }
            else
                break;
        }
    }
}

/*****************************************************************
*Function: trackMend_HalfWidth(*)
*Description: 丢左右线 半宽补中线
              左右都没丢 直接计算中线
*parameter: *
*Return: *
*****************************************************************/
void trackMend_HalfWidth(void)
{
    int8_t i;
    float err = 0, aveErr = 0;
    uint8_t count = 0;
    uint8_t centerCompensation = 0;

    //(一) 两边丢线(没救了)------------------------------------
    if (imageLine.Lost_Left && imageLine.Lost_Right)
    {
        imageLine.Lost_Center = true;
    }

    //(二) 只丢左边线
    else if (imageLine.Lost_Left)
    {
        imageLine.Lost_Center = false;

        //1. 计算中线补偿
        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Right[i])
            {
                err += (((MT9V03X_W) / 2 + roadK * i / 2 + roadB / 2) - imageLine.Point_Right[i]);
                count++;
            }
        }

        //2. 计算平均误差
        aveErr = (float)(err / count);
        //ave_err_max = MAX(aveErr, ave_err_max);//用于调参

        //3. 根据右边线补中线(路宽+中线补偿)
        if (count >= 5 && aveErr > 0)//点数足够且确定右边界线往左倾斜
            centerCompensation = LIMIT2MIN(aveErr, SingleLineLeanAveERR_MAX) / SingleLineLeanAveERR_MAX * SingleLineLeanK / 2;//补偿计算

        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Right[i])
            {
                //计算中线并限幅
                imageLine.Exist_Center[i] = true;
                //amplitudeLIMIT(i, imageLine.Point_Right[i] - centerCompensation - roadK * i / 2 - roadB / 2 - (10 * flag_forkRoad_find / 2) - flag_more_SingleLineLeanK * 30);
                amplitudeLIMIT(i, imageLine.Point_Right[i] - centerCompensation - roadK * i / 2 - roadB / 2);
            }
        }

        if (flag_forkRoad_find == 1)
        {
            //三岔情况下不进行中线突变限幅
            repairRemainLine();//用最小二乘法修复未知的中线
        }
        else
        {
            limitCenter();//对得出的中线进行突变限幅
            //repairRemainLine();//用最小二乘法修复未知的中线
        }

    }

    //(三) 只丢右边线
    else if (imageLine.Lost_Right)
    {
        imageLine.Lost_Center = false;

        //1. 计算中线补偿
        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Left[i])
            {
                err += (imageLine.Point_Left[i] - ((MT9V03X_W) / 2 - roadK * i / 2 - roadB / 2));
                count++;
            }
        }

        //2. 计算平均误差
        aveErr = (float)(err / count);
        //ave_err_max = MAX(aveErr, ave_err_max);//用于调参

        //3. 根据左边线补中线(路宽+中线补偿)
        if (count >= 5 && aveErr > 0)//点数足够且确定右边界线往左倾斜
            centerCompensation = LIMIT2MIN(aveErr, SingleLineLeanAveERR_MAX) / SingleLineLeanAveERR_MAX * SingleLineLeanK / 2;//补偿计算

        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Left[i])
            {
                //计算中线并限幅
                imageLine.Exist_Center[i] = true;
                //amplitudeLIMIT(i, imageLine.Point_Left[i] + centerCompensation + roadK * i / 2 + roadB / 2 + (10 * flag_forkRoad_find / 2) + flag_more_SingleLineLeanK * 15);
                amplitudeLIMIT(i, imageLine.Point_Left[i] + centerCompensation + roadK * i / 2 + roadB / 2);
            }
        }

        if (flag_forkRoad_find == 1)
        {
            //三岔情况下不进行中线突变限幅
            repairRemainLine();//用最小二乘法修复未知的中线
        }
        else
        {
            //limitCenter();//对得出的中线进行突变限幅
            repairRemainLine();//用最小二乘法修复未知的中线
        }
        limitCenter();//对得出的中线进行突变限幅
        //repairRemainLine();//用最小二乘法修复未知的中线

    }

    //(四)两边都没有丢线
    else if (!imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        imageLine.Lost_Center = false;

        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Left[i] && imageLine.Exist_Right[i])
            {
                if (imageLine.Point_Right[i] - imageLine.Point_Left[i] < MINRoadLen)
                    continue;
                else
                {
                    //计算中线并限幅
                    imageLine.Exist_Center[i] = true;
                    amplitudeLIMIT(i, (imageLine.Point_Left[i] + imageLine.Point_Right[i]) / 2);
                }
            }
            else if ((!imageLine.Exist_Left[i] && imageLine.Exist_Right[i]) && i < MT9V03X_H / 2)
            {
                imageLine.Exist_Center[i] = true;
                amplitudeLIMIT(i, (imageLine.Point_Right[i] - (roadK * i + roadB) / 2));
            }
            else if ((imageLine.Exist_Left[i] && !imageLine.Exist_Right[i]) && i < MT9V03X_H / 2)
            {
                imageLine.Exist_Center[i] = true;
                amplitudeLIMIT(i, (imageLine.Point_Left[i] + (roadK * i + roadB) / 2));
            }
            else continue;
        }

        limitCenter();
        repairRemainLine();

    }
}

/*****************************************************************
*Function: repairRemainLine(*)
*Description: 用最小二乘法修复未知的中线
              这里的前后是相对于摄像头的距离来说的，而不是图像
              也就是说，图像的下方被称为前(起始)
*parameter: *
*Return: *
*****************************************************************/
void repairRemainLine(void)
{
    int8_t i, j;
    uint8_t mediumLineStart = 0, mediumLineEnd = 0;

    uint8_t x1;
    uint8_t x2 = 0;

    short y1,y2;
    short mediumLine1[2][MT9V03X_H], mediumLine2[2][MT9V03X_H];//起点到拐点1的线段，拐点1到拐点2的线段

    uint8_t count1, count2;

    float k1, k2, b1, b2;

    //(一) 拟合前面部分
    //1. 先找到最下面的第一个有效中点-----------------------
    for (i = MT9V03X_H - 1; i >= 0; i--)//从下往上
    {
        if (imageLine.Exist_Center[i])
        {
            mediumLineStart = i;
            break;
        }
    }

    //2. 填补残缺点----------------------------------------
    for (i = mediumLineStart - 1; i >= 0; i--)
    {
        if (!imageLine.Exist_Center[i])//有效的起始中点的上一行中点没有
        {
            x1 = i + 1;
            y1 = imageLine.Point_Center[x1];//起始中点的下一行点

            for (j = i - 1; j >= 0; j--)//往上继续找有效中点
            {
                if (imageLine.Exist_Center[j])
                {
                    x2 = j;
                    y2 = imageLine.Point_Center[x2];
                    for (j = x2; j <= x1; j++)
                    {
                        imageLine.Exist_Center[j] = true;
                        imageLine.Point_Center[j] = (short)((y2 - y1)*(j - x1) / (x2 - x1) + y1);
                    }
                    break;
                }
            }


        }
        else
            continue;
    }

    //3. 拟合图像下方的中线-----------------------------------
    count1 = 0;
    if (mediumLineStart >= 5)
    {
        for (i = mediumLineStart; i >= mediumLineStart - 5; i--)
            if (imageLine.Exist_Center[i])
            {
                mediumLine1[0][count1] = i;
                mediumLine1[1][count1] = imageLine.Point_Center[i];
                count1++;
            }

        if (count1 > 1)
        {
            leastSquareMethod(mediumLine1[0], mediumLine1[1], count1, &k1, &b1);
            for (i = MT9V03X_H - 1; i > mediumLineStart; i--)
            {
                //int8_t temp = (int8_t)(k1*i + b1);
                imageLine.Point_Center[i] = (k1*i + b1);
                imageLine.Exist_Center[i] = true;
            }
        }
    }

    //(二)拟合后面部分-----------------------------------
    //1. 先找到图像最上面的有效中点(中线终止点)
    for (i = 0; i < MT9V03X_H; i++)
    {
        if (imageLine.Exist_Center[i])
        {
            mediumLineEnd = i;
            break;
        }
    }
    //2. 拟合图像上方的中线
    count2 = 0;
    if (mediumLineEnd < MT9V03X_H - 5)
    {
        for (i = mediumLineEnd; i <= mediumLineEnd + 5; i++)
            if (imageLine.Exist_Center[i])
            {
                mediumLine2[0][count2] = i;
                mediumLine2[1][count2] = imageLine.Point_Center[i];
                count2++;
            }

        if (count2 > 1)
        {
            leastSquareMethod(mediumLine2[0], mediumLine2[1], count2, &k2, &b2);
            for (i = 0; i < mediumLineEnd; i++)
            {
                //int8_t temp = (int8_t)(k2*i + b2);
                imageLine.Point_Center[i] = (k2*i + b2);
                imageLine.Exist_Center[i] = true;
            }
        }
    }
}

/*****************************************************************
*Function: amplitudeLIMIT(*)
*Description: 对得出的中线进行赋值限幅
              给中点赋值，并认为该行中线有效
*parameter: i 中线点所在行
            amp 中线点所在列
*Return: *
*****************************************************************/
void amplitudeLIMIT(uint8_t i, uint8_t amp)
{
    imageLine.Exist_Center[i] = true;
    imageLine.Point_Center[i] = amp;
    if(imageLine.Point_Center[i]>93)
        imageLine.Point_Center[i]=93;
    else if(imageLine.Point_Center[i]<0)
        imageLine.Point_Center[i]=0;
}

/*****************************************************************
*Function: limitCenter(*)
*Description: 对得出的中线进行突变限幅
*parameter: *
*Return: *
*****************************************************************/
void limitCenter(void)
{
    for (int i = MT9V03X_H - 2; i >= 1; i--)//从下往上
    {
        if (imageLine.Exist_Center[i] && imageLine.Exist_Center[i + 1])
        {
            if (my_abs_short(imageLine.Point_Center[i] - imageLine.Point_Center[i + 1]) > 6)
            {
                if ((imageLine.Exist_Center[i]) && (!imageLine.Exist_Center[i - 1]))
                    imageLine.Exist_Center[i] = false;
            }
        }
        if (imageLine.Exist_Center[i] && imageLine.Exist_Center[i - 1])
        {
            if (my_abs_short(imageLine.Point_Center[i] - imageLine.Point_Center[i - 1]) > 6)
            {
                if ((imageLine.Exist_Center[i]) && (!imageLine.Exist_Center[i + 1]))
                    imageLine.Exist_Center[i] = false;
            }
        }
        if (imageLine.Exist_Center[i] && !imageLine.Exist_Center[i - 1] && !imageLine.Exist_Center[i + 1])
        {
            imageLine.Exist_Center[i] = false;
        }
    }
}

/*****************************************************************
*Function: mediumLineCheck(*)
*Description: 中线校验
*parameter: *
*Return: *
*****************************************************************/
void mediumLineCheck(void)
{
    uint8_t i;

    if (!imageLine.Lost_Center)//首先中线没丢
    {
        //(一) 右丢线
        if (imageLine.Lost_Right && !imageLine.Lost_Left)
        {
            //图像上半部分左线未超过阈值且左线未越过中间则补线无效
            short leftMAX = 0;//表示左边界点横向最右延伸到了哪里
            uint8_t leftCount = 0;

            for (i = 0; i < MT9V03X_H * 2 / 3; i++)///////////////////////////////////////////////
            {
                if (imageLine.Exist_Left[i])
                {
                    leftCount++;
                    if (imageLine.Point_Left[i] > leftMAX)
                        leftMAX = imageLine.Point_Left[i];
                }
            }

            //if ((leftCount < MT9V03X_H / 6) && (leftMAX < MT9V03X_W / 3))
            if ((leftCount < MT9V03X_H / 8) && (leftMAX < MT9V03X_W / 4))
            {
                imageLine.Lost_Center = true;
            }

        }

        //(二) 左丢线
        else if (imageLine.Lost_Left)
        {
            //图像上半部分右线未超过阈值且右线未越过中间则补线无效
            short rightMIN = MT9V03X_W;//表示右边界点横向最左延伸到了哪里
            uint8_t rightCount = 0;

            for (i = 0; i < MT9V03X_H * 2 / 3; i++)
            {
                if (imageLine.Exist_Right[i])
                {
                    rightCount++;
                    if (imageLine.Point_Right[i] < rightMIN)
                        rightMIN = imageLine.Point_Right[i];
                }
            }

            //if ((rightCount < MT9V03X_H / 6) && (rightMIN > MT9V03X_W * 2 / 3))
            if ((rightCount < MT9V03X_H / 8) && (rightMIN > MT9V03X_W * 3 / 4))
            {
                imageLine.Lost_Center = true;
            }
        }

        //(三) 不丢线
        else
        {
            uint8_t len_temp;
            uint8_t len_basis;
            uint8_t lostCenter_cnt = 0;

            for (i = MT9V03X_H / 2; i < MT9V03X_H; i++)//从图像中间开始向下检测
            {
                //如果本行左右边点存在
                if (imageLine.Exist_Left[i] && imageLine.Exist_Right[i])
                {
                    len_temp = imageLine.Point_Right[i] - imageLine.Point_Left[i];
                    len_basis = roadK * i + roadB;

                    //本行实际路宽应全满或大于len_basis - 10
                    //如不满足, 无效中线并跳出循环
                    if ((len_temp < MT9V03X_W - 5)
                        && (len_temp < len_basis - 15))
                    {
                        lostCenter_cnt++;
                        //imageLine.Lost_Center = true;
                        //break;
                    }

                    if (lostCenter_cnt > 5)
                    {
                        lostCenter_cnt = 0;
                        imageLine.Lost_Center = true;
                        break;
                    }
                }
            }


        }

    }
    if(find_ring_Left)
    {
        for(int k = 0; k < 60; k++)
        {
            if(imageLine.Exist_Center[k])
                if(imageLine.Point_Center[k]>=46)
                    imageLine.Point_Center[k] = 40;
        }
    }
    if(find_ring_Right)
    {
        for(int k = 0; k < 60; k++)
        {
            if(imageLine.Exist_Center[k])
                if(imageLine.Point_Center[k]<=48)
                    imageLine.Point_Center[k] = 54;
        }
    }
}


/*****************************************************************
*Function: White_Black_White_detect(uint8_t row, uint8_t half)
*Description: 检测白黑白排布
*parameter: row：行
*           half：0左半边，1右半边，2全图
*Return: *
*****************************************************************/
bool White_Black_White_detect(uint8_t row, uint8_t half)
{
    uint8_t i,state = 0;

    switch(half)
    {
        case 0:
            for(i = 1; i < MT9V03X_W_2_3; i++)
            {
                if(state==0&& isWhite(i,row) && !isWhite(i+1,row))
                {
                    state++;
                }
                else if(state==1 && !isWhite(i,row) && isWhite(i+1,row))
                {
                    state++;break;
                }
            }
            if(state==2)
                return true;
            else
                return false;

        case 1:
            for(i = MT9V03X_W_3; i < MT9V03X_W; i++)
            {
                if(state == 0 && isWhite(i-1,row) && !isWhite(i,row))
                {
                    state++;
                }
                else if(state == 1 && !isWhite(i-1,row) && isWhite(i,row))
                {
                    state++;break;
                }
            }
            if(state == 2)
                return true;
            else
                return false;
        case 2:
            for(i = 1; i < MT9V03X_W; i++)
            {
                if(state == 0&& isWhite(i-1,row) && !isWhite(i,row))
                {
                    state++;
                }
                else if(state == 1 && !isWhite(i-1,row) && isWhite(i,row))
                {
                    state++;break;
                }
            }
            if(state == 2)
                return true;
            else
                return false;
    }

    return false;

}
volatile float center = MT9V03X_W_2;
volatile uint8_t AIM_LINE = AIM_LINE_SET;//目标行（本行所找中点作为的参考行）

/*****************************************************************
*Function: updateMediumLine(*)
*Description: 更新中线打角
*parameter: *
*Return: *
*****************************************************************/
void updateMediumLine(void)
{
    bool rst;
    static uint8_t lostTime = 0;



    //1. 寻找中线
    //ImageProcessInit();//参数初始化
    image_pre_processing();//图像预处理
    Get_White_Num(0);//获取每一行的白点个数
    trackDFS();//DFS深度遍历
    if(!flag_isRight_ring && !flag_isLeft_ring)
        forkRoad_find();//三岔路口判断
    if(flag_garage_turn==2)
        zebra_cross_detect();
    //forkRoad_turnPoint_prefind();//////////////////////////////////////斜入三岔
    //forkRoad_turnPoint_find();//////////////////////////////////////
    left_right_Limit();

    lineChangeLimit();//边界点防突变
    lost_mend();
    doFilter();//对可能的错误点进行过滤

    doMend();//补线程序
    if (flag_forkRoad_find == 0 && flag_isLeft_ring == 0 && flag_isRight_ring == 0)
        mediumLineCheck();//中线校验程序
    if(flag_garage_turn == 0)
    {
        imageLine.Lost_Center = false;
        imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 47;
        garage_find();
    }
    else if(flag_garage_turn == 1)
    {
        imageLine.Lost_Center = false;
        imageLine.Exist_Center[AIM_LINE_SET] = true;
        imageLine.Point_Center[AIM_LINE_SET] = 80;
        garage_find();
    }
    //2. 计算中线打角
    rst = MediumLineCal(camERR.cam_finalCenterERR);

    //3. 中线打角
    if (rst)//摄像头打角
    {
        lostTime = 0;
        //flag.turnWAY = 0;
    }
    else if (!rst && (lostTime <= 2))//摄像头打角
    {
        lostTime++;
    }
}

/*****************************************************************
*Function: MediumLineCal(*)
*Description: 计算中线打角
              关于误差的计算还要测试！！！
*parameter: *
*Return: *
*****************************************************************/
bool MediumLineCal(volatile short * final)
{
    uint8_t i;
    static float lastCenter = 47;//94*60  理论中线是47

    AIM_LINE = AIM_LINE_SET;

    if (imageLine.Exist_Center[AIM_LINE] == false && !imageLine.Lost_Center)
    {
        //假如控制行处中线不存在，往下找个点
        for (i = AIM_LINE; i < AIM_LINE+25; i++)
        {
            if (imageLine.Exist_Center[i])
            {
                AIM_LINE = i;
                break;
            }
        }
        if(AIM_LINE==AIM_LINE_SET)
        {
            for (i = AIM_LINE; i > AIM_LINE-20; i--)
            {
                if (imageLine.Exist_Center[i])
                {
                    AIM_LINE = i;
                    break;
                }
            }
        }
    }

    if (imageLine.Exist_Center[AIM_LINE])
    {
        if ((my_abs_float(lastCenter - (float)imageLine.Point_Center[AIM_LINE]) > 50.0f) && (!flag_forkRoad_find))
        {
            lastCenter = imageLine.Point_Center[AIM_LINE];
            return false;
        }
        else
        {
            //更新中线误差
            for (i = 0; i < 2; i++)
                *(final + 2 - i) = *(final + 1 - i);

            //计算最新中线误差
            *(final) = (imageLine.Point_Center[AIM_LINE] - center);

            lastCenter = imageLine.Point_Center[AIM_LINE];
        }
    }
    return 1;
}







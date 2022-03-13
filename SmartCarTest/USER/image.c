#include <image.h>
#include <mymath.h>
#include <stdint.h>
#include <stdio.h>
#include <LQ_UART.h>
#include <LQ_STM.h>
#include <TSY_WIFI.h>

uint8_t mt9v03x_image[MT9V03X_W][MT9V03X_H];//ԭʼͼ��track_boundary_k_left

uint8_t bin_thr = 0;//��ֵ����ֵ
volatile imageLine_t imageLine;
volatile camERR_t camERR;

volatile int ave_err_max = 0;//���ڵ���
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
float track_boundary_k_right[3] = { 0,0,0 };//��¼������ �ұ߽�б��
float track_boundary_k_left[3] = { 0,0,0 };//��¼���߶����� ��߽�б��
float track_boundary_k_err[2] = { 0,0 };
short flag_more_SingleLineLeanK = 0;

short isLeftToForkRoad = 1;//����ǰ��ת���׽������ұ�; ����ǰ��ת���׽��������
short isForkRoadTurnLeft = 0;//����ת���� 1 ��ת; 0 ��ת

/*****************************************************************
*Function: isWhite(*)
*Description: �ж��Ƿ�Ϊ�׵�
*parameter: row ͼ����
            line ͼ����
*Return: true �׵�
         false �ڵ�
*****************************************************************/
bool isWhite(short line, short row)
{
    //�����ж�
    if (!(row >= 0 && row < MT9V03X_H && line >= 0 && line < MT9V03X_W))
        return false;

    //�жϰ׵�ڵ�
    if (Bin_Pixle[line][row])
        return true;
    else
        return false;
}


/*****************************************************************
*Function: isLeftPoint(*)
*Description: �ж��Ƿ�Ϊ��߽��
*parameter: i ���ص���
            j ���ص���
*Return: true ����߽��
         false ������߽��
*****************************************************************/
bool isLeftPoint(short i, uint8_t j)
{
    if (i < 2 || i >= MT9V03X_W - 2 || j >= MT9V03X_H)//ͼ���Ե
        return false;
    //�ұ�һ�����ܳ�������
    if (((!isWhite(i, j)) || (!isWhite(i + 1, j)) || (!isWhite(i + 2, j)) || (!isWhite(i + 3, j)) ))
        return false;
    //���һ�����ܳ���·
    if (isWhite(i - 1, j) || isWhite(i - 2, j) || isWhite(i - 3, j) || isWhite(i - 4, j))
        return false;

    return true;
}

/*****************************************************************
*Function: isRightPoint(*)
*Description: �ж��Ƿ�Ϊ�ұ߽��
*parameter: i ���ص���
            j ���ص���
*Return: true ���б߽��
         false �����ұ߽��
*****************************************************************/
bool isRightPoint(short i, uint8_t j)
{
    if (i < 2 || i >= MT9V03X_W - 2 || j >= MT9V03X_H)//ͼ���Ե
        return false;
    //���һ�����ܳ�������
    if (((!isWhite(i, j)) || (!isWhite(i - 1, j)) || (!isWhite(i - 2, j)) || (!isWhite(i - 3, j))))
        return false;
    //�ұ�һ�����ܳ���·
    if (isWhite(i + 1, j) || isWhite(i + 2, j) || isWhite(i + 3, j) || isWhite(i + 4, j))
        return false;

    return true;
}

/*****************************************************************
*Function: isEdgePoint(*)
*Description: �����ھŹ�������(�����������߽��������£����¸��߽�������)
*parameter: i ���ص���
            j ���ص���
*Return: true �Ǳ߽��
         false ���Ǳ߽��
*****************************************************************/
bool isEdgePoint(short i, uint8_t j)
{
    if (j < 1 || j >= MT9V03X_H - 1 || i<2 || i>MT9V03X_W - 3)//ͼ���Ե
        return false;
    else if (
        ((isWhite(i, j)))//�����ǰ�ɫ
        && ((!isWhite(i + 1, j)) || (!isWhite(i - 1, j)) || (!isWhite(i, j + 1))
            || (!isWhite(i, j - 1)))//��������������һ����ɫ
        )
        return true;
    else
        return false;
}

int White_Num_Left = 0;
int White_Num_Right = 0;
/*****************************************************************
*Function: Get_White_Num(uint8_t mode)
*Description: �õ��׵����
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
static short stackTopPos;//ջ��ָ��
static uint8_t roadPointStackX[TOTAL_POINT];//DFS��ջ�洢Ѱ��X����
static uint8_t roadPointStackY[TOTAL_POINT];//DFS��ջ�洢Ѱ��Y����
static bool isVisited[MT9V03X_W][MT9V03X_H];//���ص��Ƿ���ʹ�

/*****************************************************************
*Function: get_bin_thr(*)
*Description: ��ȡ��ֵ����ֵ��ƽ������
*parameter: *
*Return: *
*****************************************************************/
void get_bin_thr(void)//��ȡ��ֵ����ֵ��ƽ������
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
*Description: �Ҷ�ͼ���ֵ��
*parameter: *
*Return: *
*****************************************************************/
void gray2bin(void)
{
    for (int i = 0; i < MT9V03X_W; i++)
    {
        for (int j = 0; j < MT9V03X_H; j++)
        {
            if (mt9v03x_image[i][j] > bin_thr)//�׵� 1���ڵ� 0
                Bin_Pixle[i][j] = true;
            else
                Bin_Pixle[i][j] = false;
        }
    }
}

/*****************************************************************
*Function: Pixle_Filter(*)
*Description: �˳���ֵ��ͼ�����
              ����������ɫ����ɫ���������Ľ��й������
*parameter: *
*Return: *
*****************************************************************/
void Pixle_Filter(void)
{
    int nr; //��
    int nc; //��

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
*Description: ��֤ͬһ����߽��һ�����ұ߽�����
*parameter: *
*Return: *
*****************************************************************/
void left_right_Limit(void)
{
    short i = 0;

    for (i = 1; i < MT9V03X_H - 1; i++)//�����ϵ�����
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


//�������
uint8_t fork_Point_row = 0;
uint8_t fork_Point_line = 0;
uint8_t fork_Ypoint_Left = 0;
uint8_t fork_Ypoint_Right = 0;

/*****************************************************************
*Function: forkRoad_find(*)
*Description: ����·���жϣ���գ�
*             ���˼·��
*             1�����Y��·�ڣ���⵽�����¹յ��붥�˲�·����
*             2����δ��⵽���ж����¹յ��Ƿ���ڣ������ڣ�һ����ڶ����յ��Ҳ��⵽���ߣ��˳���Щ���ߣ��������½��붥�˹յ�����
*             3����12��δ��⵽�������Ƿ����ߣ��������ߣ����ⶥ�˲�·�����˲�·�Ų�Ϊ�׺ڰף�������������8�У��������붥�˹յ�����
*parameter: *
*Return: *
*****************************************************************/
void forkRoad_find(void)
{
    uint8_t i;
    uint8_t turnPoint_Left = 0;
    uint8_t turnPoint_Right = 0;
    for (i = FORK_DETECT_START_ROW; i > FORK_DETECT_END_ROW; i--)//��������
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

                //�����ж�����
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

                //�����ж�����
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
*Description: ���������˳�һ�ߵı߽��
*parameter: select 0,�˳��ұ߽�㣻1,�˳���߽��
*Return: *
*****************************************************************/
void forkRoad_in_filter(short select)
{
    short i = 0;

    if (select == 0)
    {
        //imageLine.Lost_Right = true;
        for (i = fork_Ypoint_Left; i > EFFECTIVE_ROW; i--)//��������
        {
            if (imageLine.Exist_Left[i])
                imageLine.Exist_Left[i] = false;
        }
    }
    else if (select == 1)
    {
        //imageLine.Lost_Left = true;
        for (i = fork_Ypoint_Right; i > EFFECTIVE_ROW; i--)//��������
        {
            if (imageLine.Exist_Right[i])
                imageLine.Exist_Right[i] = false;
        }
    }
}

/*****************************************************************
*Function: forkRoad_mend(*)
*Description: �����߽��
*parameter: select 0,����߽�� ��ת��1,���ұ߽�� ��ת ֻд����ת
*Return: *
*****************************************************************/
void forkRoad_mend(short select)
{
    uint8_t i = 0;

    //�˳���һ�ߵı߽��
    forkRoad_in_filter(select);
    //flag_crossLeft_find=0;
    //flag_crossRight_find=0;
    //crossRight_flag=0;
    //crossLeft_flag=0;
    //������
    if (select == 0)
    {
        //����߽�㣨ͼ���Ұ벿�֣���ת��
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
        if (count == 5)//��5���㼴�ɿ�ʼ����
        {
            leastSquareMethod(MendBasis_left[0], MendBasis_left[1], 5, &k_left, &b_left);

            //��ʼ����
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
        //���ұ߽�㣨ͼ����벿�֣���ת��
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
        if (count == 5)//��5���㼴�ɿ�ʼ����
        {
            leastSquareMethod(MendBasis_right[0], MendBasis_right[1], 5, &k_right, &b_right);

            //��ʼ����
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
*Description: ����ֱ�ߺ����ж�
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

            if (black_line_count > 7)//����5��ȫ��
            {
                flag_garage_turn = 1;
                //flag_garage_detect = 0;//��⵽һ��֮��Ͳ������
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
        if (flag_mend)//����5��ȫ��
        {
            flag_garage_turn = 2;
            //flag_garage_detect = 0;//��⵽һ��֮��Ͳ������
            garage_lost = 0;
        }
    }
}

uint8_t zebra_cross_count = 0;  //��������Ч����
uint8_t lock_zebra = 0; //�Ƿ񲻼�������
uint8_t is_zebra = 0;   //�ж��Ƿ���ڰ�����
uint8_t garage_in = 0;  //����жϱ�־
/*****************************************************************
*Function: zebra_cross_detect(*)
*Description: ��������
*             ���׺ڣ�WB���ͺڰף�BW�����򣬼�⵽����++���ﵽ7�����ϼ��ж�Ϊ������
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
*Description: ��ⲹ��
*          ֱ�Ӹ�ƫ��
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
        for(uint8_t i = MT9V03X_H_2; i < MT9V03X_H_2_3; i++)//���ڵ�������⵽ͣ��
        {
            if(imageLine.White_Num[i] < 10)
            {
                manControl = 1;
                break;
            }
        }
    }
}




bool isLeftLineStraight = true;//�����Ƿ�Ϊֱ��
bool isRightLineStraight = true;//�����Ƿ�Ϊֱ��
static bool isLeftRightLineExist = true;//����׼������ұ߽��Ƿ����
uint8_t find_ring_Left = 0; //�󻷽�����־
uint8_t find_ring_Right = 0;//�һ�������־
uint8_t find_ring_flag_Right = 0;//�һ�����־
uint8_t find_ring_flag_Left = 0;//�󻷼���־
uint8_t flag_isLeft_ring = 0;//�󻷻��ڱ�־���������
uint8_t flag_isRight_ring = 0;//�һ����ڱ�־���������
uint8_t isLeft = 0, isRight = 0;//��ʼ������һ���־
uint8_t startTick = 0;//��ʼ��ʱ����ʱ������ڱ�־
static uint8_t ring_in_mend_count = 0; //û��⵽�뻷���߹յ����
static uint8_t ring_in_lost_left = 0;  //û��⵽���󻷲��߱�־
static uint8_t ring_in_lost_right = 0; //û��⵽���һ����߱�־
static uint8_t ring_out_turnPoint_row = EFFECTIVE_ROW;
short ring_out_turnPoint_line = 0;

/*�������
 * ���׺ڰ�
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
*Description: ���Բ��
* *          �һ�Ϊ��
*              |   |
*  *�ж��һ�->   |   |/-----\
*              |       --  \
*  *�׶�3->     |      /  \ |
*  *�׶�2->     |      \  / |
*  *�׶�1->     |       --  /
*  *��ʼ�ж�->   |   |\-----/
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
    //�ҹյ��ж���Բ��������Բ�����жϷ���Ϊ һ��Ϊ ��->��->�� ����һ��Ϊֱ��
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
    /*���˼·
            1�����·��ͻ��
            2�����·���խ
            3�����·����
            4�����յ�
            5��ת��
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
*Description: �뻷����
*parameter: *
*Return: *
*****************************************************************/
void ring_in_Mend(void)
{
    uint8_t i;
    uint8_t road_Bottom_Left = 0, road_Bottom_Right = 0;
    //����˼·
    //���·��ͻ�䣬���·�ʱ�ȼ��׺ڰף���⵽���ߣ�δ��⵽ǿ�Ƹ�ƫ�3֡�������ж��뻷���
    if(find_ring_Left)
    {
        for(i = RING_IN_MEND_START_ROW; i > RING_IN_MEND_END_ROW; i--)//��������
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
        //�͹յ�����
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

        //��ȫ�Ϸ�����
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
        if (count == 5)//��5���㼴�ɿ�ʼ����
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
            //��ʼ����
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
        if (count == 5)//��5���㼴�ɿ�ʼ����
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
            //��ʼ����
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
*Description: �����ҹյ㲢�˳��յ�����ı߽��
              ʹ���貹��Բ��״̬������
              ����������а׵���
*parameter: *
*Return: *
*****************************************************************/
void ring_out_turnPoint_filter_mend(void)
{
    uint8_t i = 0, j = 0;//���幤�߱���
    short flag_leftRing_Lost = 0;
    short flag_rightRing_Lost = 0;

    //�ָ���ʼ��
    ring_out_turnPoint_row = EFFECTIVE_ROW;
    ring_out_turnPoint_line = 0;
    short count;
    count = 0;
    //��Բ��
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
            if (count == 5)//��5���㼴�ɿ�ʼ����
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
                //��ʼ����
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
    //��Բ��
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
            if (count == 5)//��5���㼴�ɿ�ʼ����
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
                //��ʼ����
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
*Description: ���뻷���߲���
              ֱ�Ӹ�ƫ��
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
*Description: ������Բ����־λ
*parameter: way��0�� 1�һ�
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
*Description:  ������·�ڣ������ǻ����ֲ洦��գ���ֹ����
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
                if (count == 5)//��5���㼴�ɿ�ʼ����
                {
                    leastSquareMethod(MendBasis[0], MendBasis[1], 5, &k, &b);
                    //��ʼ����
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
             if (count == 5)//��5���㼴�ɿ�ʼ����
             {
                 leastSquareMethod(MendBasis[0], MendBasis[1], 5, &k, &b);
                 //��ʼ����
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
*Description: ȫʮ�ּ��
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
*Description: ʮ�ּ���һ��
*        -----
*       /     \
*       |  ��  |       ------
*       \      |       �� ����ͷ��ʾ��Χ
*        ---|  |       ��
*           |  |       ------
*        -----
*       /     \
*       |  ��  |      ------
*       |     /       �� ����ͷ��ʾ��Χ
*       |  |--        ��
*       |  |          -------
*parameter: 0����ʮ�� 1����ʮ��
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
*Description: ʮ�ּ����
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
*Description: �ڶ�����⣺��ʮ�ּ��
*                      ------
*        -----         ��
*       /     \         ����ͷ��ʾ��Χ
*       |  ��  |       ��
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
*Description: �ڶ���ʮ�ּ�⣺��ʮ�ּ��
*        -----         ------
*       /     \        �� ����ͷ��ʾ��Χ
*       |  ��  |       ��
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
*Description: ��ʮ�� ���ߣ�ֱ�Ӹ�ƫ�
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
*Description: δ��⵽ʮ��ʱ�����־λ
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
*Description: ͼ��Ԥ������(�������Ԥ������)
*parameter: *
*Return: *
*****************************************************************/
void image_pre_processing(void)
{
    Pixle_Filter();//���˶�ֵ��ͼ�����
    left_right_Limit();//��֤ͬһ����߽��һ�����ұ߽�����
}

/*****************************************************************
*Function: image_pre_processing(*)
*Description: ͼ������ر�����ʼ��
*parameter: *
*Return: *
*****************************************************************/
void ImageProcessInit(void)
{
    uint8_t i, j;
    //���ߺ������߶�������
    imageLine.Lost_Center = true;
    imageLine.Lost_Left = true;
    imageLine.Lost_Right = true;

    stackTopPos = -1;//ջ��ָ���ֵ

    for (i = 0; i < MT9V03X_H; i++)
    {
        //ÿһ�е����ұ߽������ĵ㶼������
        imageLine.Exist_Left[i] = false;
        imageLine.Exist_Right[i] = false;
        imageLine.Exist_Center[i] = false;

        //�߽������ĵ���Ϊ��ʼλ��
        imageLine.Point_Left[i] = 1;
        imageLine.Point_Right[i] = MT9V03X_W - 1;
        imageLine.Point_Center[i] = MT9V03X_W / 2;

        //ÿһ�еİ׵��������
        imageLine.White_Num[i] = 0;

        for (j = 0; j < MT9V03X_W; j++)
        {
            isVisited[j][i] = false;//DFS��, ���е㶼��û��������
        }
    }

    camERR.cam_finalCenterERR[0] = 0;
    camERR.cam_finalCenterERR[1] = 0;
    camERR.cam_finalCenterERR[2] = 0;
    camERR.K_cam = 1.0f;
}

/*****************************************************************
*Function: trackDFS(*)
*Description: DFSѲ��
              ������������ҳ���ͨ�������а׵�(DFS������)
*parameter: *
*Return: *
*****************************************************************/
void trackDFS(void)
{
    uint8_t i, j;

    //ѡ��ͼƬ�·��е���Ϊ��ʼ��
    if (isWhite(MT9V03X_W / 2, MT9V03X_H - 2))//���·��е���ǰ׵�
    {
        stackTopPos++;
        roadPointStackX[stackTopPos] = MT9V03X_W / 2;
        roadPointStackY[stackTopPos] = MT9V03X_H - 2;
        isVisited[MT9V03X_W / 2][MT9V03X_H - 2] = true;
    }
    else
    {
        for (i = MT9V03X_W - 3; i >= 2; i--)//������������
        {
            if (isWhite(i - 2, MT9V03X_H - 2) && isWhite(i - 1, MT9V03X_H - 2) && isWhite(i, MT9V03X_H - 2)
                && isWhite(i + 1, MT9V03X_H - 2) && isWhite(i + 2, MT9V03X_H - 2)//����5���׵�
                )
            {
                //����������ջ
                stackTopPos++;//stackTopPos����ͱ�ʾջ�ǿ�
                roadPointStackX[stackTopPos] = i;
                roadPointStackY[stackTopPos] = MT9V03X_H - 2;
                isVisited[i][MT9V03X_H - 2] = true;
                break;
            }
        }
    }

    while (stackTopPos >= 0)
    {
        //��ջ
        i = roadPointStackX[stackTopPos];
        j = roadPointStackY[stackTopPos];
        stackTopPos--;

        //������磬ֱ��continue
        if ((j < EFFECTIVE_ROW) || (j > MT9V03X_H - 2) || (i < 1) || (i > MT9V03X_W - 1))
        {
            continue;
        }

        /*************���²���ԭ���ǣ����׵���ջ�����ڵ�����ж�Ϊ�߽��(��������Ҫ�����˲����ߵȲ���)**************/
        //��ջ�洢��...����(��)����(��)...
        //һ������£��ϵİ׵���ȳ�ջ(����Ϊ��һֱ��ͼ���Ϸ�����)��Ȼ���ٳ�����
        //��������
        if (!isVisited[i - 1][j])
        {
            if (isWhite(i - 1, j))
            {
                //�׵���ջ
                stackTopPos++;
                roadPointStackX[stackTopPos] = i - 1;
                roadPointStackY[stackTopPos] = j;
                isVisited[i - 1][j] = true;
            }
            else
            {
                //�ڵ�����ж�Ϊ�߽��
                if (isLeftPoint(i, j))
                {
                    imageLine.Point_Left[j] = i;//���߹켣
                    imageLine.Exist_Left[j] = true;
                }
            }
        }

        //��������
        if (!isVisited[i + 1][j])
        {
            if (isWhite(i + 1, j))
            {
                stackTopPos++;
                roadPointStackX[stackTopPos] = i + 1;
                roadPointStackY[stackTopPos] = j;
                isVisited[i + 1][j] = true;
            }
            else//�����ҵ��ұ߽�
            {
                if (isRightPoint(i, j))
                {
                    imageLine.Point_Right[j] = i;//���߹켣
                    imageLine.Exist_Right[j] = true;
                }
            }
        }

        //��������(���ϲ��жϱ߽��)
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
*Description: �߽�㲻ͻ��
*parameter: *
*Return: *
*****************************************************************/
void lineChangeLimit(void)
{
    short i, j;
    float leftK = 0;
    float rightK = 0;

    //��߽���������Ч��б�ʼ��
    for (i = MT9V03X_H - 2; i > 0; i--)//��������
    {
        if (imageLine.Exist_Left[i])//���ҵ���һ����Ч��
        {
            for (j = i + 1; j < MT9V03X_H; j++)//���������ٽ���Ч��
            {
                if (imageLine.Exist_Left[j])
                {
                    leftK = getLineK(i, imageLine.Point_Left[i], j, imageLine.Point_Left[j]);

                    if (ABS(leftK) > K_MAX_THRESHOLD)
                    {
                        imageLine.Exist_Left[i] = false;
                        //imageLine.Exist_Left[j] = false;
                    }
                    break;//ֻҪ�ҵ�һ���ٽ���Ч�㣬�����break����һ��i
                }
                else
                    continue;
            }
        }
        else
            continue;
    }

    //�ұ߽���������Ч��б�ʼ��
    for (i = MT9V03X_H - 2; i > 0; i--)//��������
    {
        if (imageLine.Exist_Right[i])//���ҵ���һ����Ч��
        {
            for (j = i + 1; j < MT9V03X_H; j++)//���������ٽ���Ч��
            {
                if (imageLine.Exist_Right[j])
                {
                    rightK = getLineK(i, imageLine.Point_Right[i], j, imageLine.Point_Right[j]);

                    if (ABS(rightK) > K_MAX_THRESHOLD)
                    {
                        imageLine.Exist_Right[i] = false;
                        //imageLine.Exist_Right[j] = false;
                    }
                    break;//ֻҪ�ҵ�һ���ٽ���Ч�㣬�����break����һ��i
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
*Description: ����filter�������
*parameter: *
*Return: *
*****************************************************************/
void doFilter(void)
{
    lostLine_Filter();//��Ч�й�����ȥ(�������ж�)
    position_Filter();//λ�ò�����ȥ

    //slope_Filter();//б�ʲ�����ȥ

    lostLine_Filter();
    if(flag_isLeft_ring || flag_isRight_ring)
    singlePoint_Filter();//���ԣ���������ȥ

}

/*****************************************************************
*Function: lostLine_Filter(*)
*Description: ���ұ�����Ч�й������ж϶�����
*parameter: *
*Return: *
*****************************************************************/
void lostLine_Filter(void)
{
    //������߽��ߵ��ж�--------------------
    uint8_t count = 0;
    uint8_t i = 0;

    for (i = 0; i < MT9V03X_H; i++)//���ϵ�������
    {
        if (imageLine.Exist_Left[i] == true)
            count++;
    }

    if (count < VALID_LINE_THRESHOLE)//�����Ч�г�����ֵ��Ϊ�ñ߽��߶�ʧ
    {
        imageLine.Lost_Left = true;
        for (i = 0; i < MT9V03X_H; i++)//���ϵ�������
        {
            imageLine.Exist_Left[i] = false;
        }
    }
    else
        imageLine.Lost_Left = false;

    //�����ұ߽��ߵ��ж�--------------------
    count = 0;
    for (i = 0; i < MT9V03X_H; i++)
    {
        if (imageLine.Exist_Right[i] == true)
            count++;
    }

    if (count < VALID_LINE_THRESHOLE)//�����Ч�г�����ֵ��Ϊ�ñ߽��߶�ʧ
    {
        imageLine.Lost_Right = true;
        for (i = 0; i < MT9V03X_H; i++)//���ϵ�������
        {
            imageLine.Exist_Right[i] = false;
        }
    }
    else
        imageLine.Lost_Right = false;
}

/*****************************************************************
*Function: position_Filter(*)
*Description: ��λ�ò��Եı�����ȥ
              �����ұߵ�����ߡ�. ���˵�Խ���м�ı߽��
              ����Ҫע�⣬���һ�߶��ߣ���һ�߿��ܻ��ܵ�����ȥ�ģ�
*parameter: *
*Return: *
*****************************************************************/
void position_Filter(void)
{
    uint8_t i;

    //�������Ҷ������ߵ�ʱ�����
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
*Description: �˳�б�ʲ��Եı߽��
              �������Ҫ����ʮ�ִ������棬�����Ҳ���ʮ�ֵĹյ�
*parameter: *
*Return: *
*****************************************************************/
void slope_Filter(void)
{
    short i, j;
    float leftK = 0;
    float rightK = 0;

    //��߽���������Ч��б�ʼ��
    for (i = MT9V03X_H - 2; i > 0; i--)//��������
    {
        if (imageLine.Exist_Left[i])//���ҵ���һ����Ч��
        {
            for (j = i + 1; j < MT9V03X_H; j++)//���������ٽ���Ч��
            {
                if (imageLine.Exist_Left[j])
                {
                    leftK = getLineK(i, imageLine.Point_Left[i], j, imageLine.Point_Left[j]);
                    if (leftK > 0 || ABS(leftK) > K_MAX_THRESHOLD)
                    {
                        //imageLine.Exist_Left[i] = false;
                        imageLine.Exist_Left[j] = false;
                    }
                    break;//ֻҪ�ҵ�һ���ٽ���Ч�㣬�����break����һ��i
                }
                else
                    continue;
            }
        }
        else
            continue;
    }

    //�ұ߽���������Ч��б�ʼ��
    for (i = MT9V03X_H - 2; i > 0; i--)//��������
    {
        if (imageLine.Exist_Right[i])//���ҵ���һ����Ч��
        {
            for (j = i + 1; j < MT9V03X_H; j++)//���������ٽ���Ч��
            {
                if (imageLine.Exist_Right[j])
                {
                    rightK = getLineK(i, imageLine.Point_Right[i], j, imageLine.Point_Right[j]);

                    if (rightK < 0 || ABS(rightK) > K_MAX_THRESHOLD)//���������״̬��һֱ���ҵ�(xzl: �����Ӱ�쵽�ҹյ㣿)
                    {
                        //imageLine.Exist_Right[i] = false;
                        imageLine.Exist_Right[j] = false;
                    }
                    break;//ֻҪ�ҵ�һ���ٽ���Ч�㣬�����break����һ��i
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
*Description: �������ڵı߽���˳�
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
*Description: ���ĵ㲻ͻ��
*parameter: *
*Return: *
*****************************************************************/
void centerChangeLimit(void)
{
    short i, j;

    for (i = MT9V03X_H - 2; i > EFFECTIVE_ROW; i--)//��������
    {
        if (imageLine.Exist_Center[i])//�ҵ�һ�����ĵ�
        {
            for (j = i + 1; j < MT9V03X_H; j++)//���������ٽ����ĵ�
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
*Description: ����ҷ�Ԫ��ʱ���õ�·����
*parameter: row����
*           way��0��·��=��������->�ҷ���һ���ڵ�
*               1��·��=�����Ļ��һ���׵�->�ҷ���һ���ڵ�
*               2��·��=ȫ���󷽵�һ���׵�->�ҷ���һ���ڵ�
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
*Description: �����Ԫ��ʱ���õ�·����
*parameter: row����
*           way��0��·��=�ұ������->�󷽵�һ���ڵ�
*               1��·��=�Ұ���Ļ��һ���׵�->�󷽵�һ���ڵ�
*               2��·��=ȫ���ҷ���һ���׵�->�󷽵�һ���ڵ�
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
*Description: ��������������ƫ���Ƿ�С��2
*parameter: row����
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
*Description: ����������ұ���ƫ���Ƿ�С��2
*parameter: row����
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

            tempPointer1 = i;//�������ҵ���߽��
            tempPointer_Val1 = imageLine.Point_Left[i];

            while (!imageLine.Exist_Left[tempPointer1 + 1])
            {
                //����
                if (isEdgePoint(tempPointer_Val1 - 1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    tempPointer_Val1 = tempPointer_Val1 - 1;
                    imageLine.Exist_Left[tempPointer1] = true;
                    imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
                }
                //��
                else if (isEdgePoint(tempPointer_Val1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    //tempPointer_Val1 = tempPointer_Val1;
                    imageLine.Exist_Left[tempPointer1] = true;
                    imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
                }
                //��
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

            tempPointer1 = i;//�������ҵ���߽��
            tempPointer_Val1 = imageLine.Point_Right[i];

            while (!imageLine.Exist_Right[tempPointer1 + 1])
            {
                //����
                if (isEdgePoint(tempPointer_Val1 + 1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    tempPointer_Val1 = tempPointer_Val1 + 1;
                    imageLine.Exist_Right[tempPointer1] = true;
                    imageLine.Point_Right[tempPointer1] = tempPointer_Val1;
                }
                //��
                else if (isEdgePoint(tempPointer_Val1, tempPointer1 + 1))
                {
                    tempPointer1 = tempPointer1 + 1;
                    //tempPointer_Val1 = tempPointer_Val1;
                    imageLine.Exist_Right[tempPointer1] = true;
                    imageLine.Point_Right[tempPointer1] = tempPointer_Val1;
                }
                //��
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
//�ڲ�����

//�ڲ�����
static void StraightLineJudge(void);//�Ź��񲹱߽��
static void trackMend_startPart(void);//��ʼ�β���
static void trackMend_endPart(void);//�����β���
static void trackMend_HalfWidth(void);//���߰����
static void repairRemainLine(void);//��С���˷��޸�δ֪������
static void amplitudeLIMIT(uint8_t i, uint8_t amp);//�Եó������߽��и�ֵ�޷�
static void limitCenter(void);//�Եó������߽���ͻ���޷�

/*****************************************************************
*Function: doMend(*)
*Description: ���в��ߺ������
*parameter: *
*Return: *
*****************************************************************/
void doMend(void)
{

        StraightLineJudge();//Ϊ����ĺ�����׼��
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
            trackMend_startPart();//��ǰ��(���복)


        if ((!isLeftLineStraight && !imageLine.Lost_Left && imageLine.Lost_Right)
            || (!isRightLineStraight && imageLine.Lost_Left && !imageLine.Lost_Right)
            || (!isLeftLineStraight && !isRightLineStraight && !imageLine.Lost_Left && !imageLine.Lost_Right)
            || !isLeft || !isRight)
            trackMend_endPart();//��ĩ��(���복)

        if (flag_forkRoad_find == 1 && !isLeftLineStraight && !isRightLineStraight && !flag_isRight_ring && !flag_isLeft_ring)
            forkRoad_mend(isForkRoadTurnLeft);//////////////////////////////////////////////�����˵��벹�ߣ�����
        else
            flag_forkRoad_find = 0;



        //track_boundary_detect();

        trackMend_HalfWidth();//�����߰����������ֱ�Ӽ���

        ring_LostCenter_mend();//�������߲���
        crossout_mend();
        if(garage_in)
            garage_in_mend();

    //centerChangeLimit();//���ĵ㲻ͻ��
}

/*****************************************************************
*Function: track_boundary_detect(*)
*Description: ���߶���ʱ�ж������߽��б��
*parameter: *
*Return: *
*****************************************************************/
void track_boundary_detect(void)
{
    short i = 0, j = 0;
    short k_count = 0;

    //������ߣ����ж��ұ��ߵ�б��
    if (imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        k_count = 0;
        for (i = EFFECTIVE_ROW; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Right[i] && (imageLine.Point_Right[i] > 0 && imageLine.Point_Right[i] < 94))//ֻȡ��ͼƬ�ڲ��ı߽����м���
            {
                if (imageLine.Exist_Right[i + 7] && imageLine.Exist_Right[i + 7 - 3])//ͼƬ����ʼ�߽������7�н��м���
                {
                    track_boundary_k_right[k_count] = ((float)imageLine.Point_Right[i + 7] - (float)imageLine.Point_Right[i + 7 - 3]) / 3;
                    k_count++;

                    if (k_count >= 3)
                        break;
                }
            }
        }
        if (k_count < 3)//û�ҹ�����б��
        {
            flag_more_SingleLineLeanK = 0;
            return;
        }
    }

    //���ұ��ߣ����ж�����ߵ�б��
    else if (!imageLine.Lost_Left && imageLine.Lost_Right)
    {
        k_count = 0;
        for (i = EFFECTIVE_ROW; i < MT9V03X_H; i++)
        {
            if (imageLine.Exist_Left[i] && (imageLine.Point_Left[i] > 0 && imageLine.Point_Left[i] < 94))//ֻȡ��ͼƬ�ڲ��ı߽����м���
            {
                if (imageLine.Exist_Left[i + 7] && imageLine.Exist_Left[i + 7 - 3])//ͼƬ����ʼ�߽������7�н��м���(�����ʼб��Ҳ��Ƚϴ�)
                {
                    track_boundary_k_left[k_count] = ((float)imageLine.Point_Left[i + 7] - (float)imageLine.Point_Left[i + 7 - 3]) / 3;
                    k_count++;

                    if (k_count >= 3)
                        break;
                }
            }
        }
        if (k_count < 3)//û�ҹ�����б��
        {
            flag_more_SingleLineLeanK = 0;
            return;
        }
    }

    //���߶�û�ж���
    else if (!imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        flag_more_SingleLineLeanK = 0;
        return;
    }

    //ǰ��û��return˵���ҵ�������б��
    //�������
    if (imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        track_boundary_k_err[0] = 0;
        track_boundary_k_err[1] = 0;

        //б�ʴ�С���
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

        //û��return˵��б�ʴ�С����
        track_boundary_k_err[0] = my_abs_short(track_boundary_k_right[1] - track_boundary_k_right[0]);
        track_boundary_k_err[1] = my_abs_short(track_boundary_k_right[2] - track_boundary_k_right[1]);
    }
    //���ұ���
    else if (!imageLine.Lost_Left && imageLine.Lost_Right)
    {
        track_boundary_k_err[0] = 0;
        track_boundary_k_err[1] = 0;

        //б�ʴ�С���
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

        //û��return˵��б�ʴ�С����
        track_boundary_k_err[0] = my_abs_short(track_boundary_k_left[1] - track_boundary_k_left[0]);
        track_boundary_k_err[1] = my_abs_short(track_boundary_k_left[2] - track_boundary_k_left[1]);
    }
    else
    {
        flag_more_SingleLineLeanK = 0;
        return;
    }

    //ǰ��û��return˵��б��֮���ƫ��Ҳ������
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
*Description: ����׼��
              ����С���˷�����ֱ�ߣ���ʵ�ʵı߽�����Ƚ�
              ��errС���趨ֵ��ʱ����Ϊ��ʱ�ı߽�����ֱ��
              ������ұ߽�Ƚ�ֱ�Ͳ���Ҫ�Ź���Ѱ����
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

    bool temp_leftLine_lost = false;//�����ߣ���ʱ��־
    bool temp_rightLine_lost = false;//�����ߣ���ʱ��־

    //�������---------------------
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

    //�������---------------------
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

    //��һ����Ϻ��ж϶������---���ж���߽綪��û�� Ҫ�Ƕ��˾�������ϣ���ͬ���ж��ұ߽�
    //������ұ��߶�û�ж����Ͳ���������Ķ������
    //*************************************************
    //1. ֻ���������
    if (imageLine.Lost_Left && !imageLine.Lost_Right)
    {
        temp_leftLine_lost = true;
        //���߶��� ����һ������----------
        for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)//��������
        {
            if (imageLine.Exist_Right[i])//�ҵ��ұ߽��
            {
                for (j = imageLine.Exist_Right[i] - 1; j > 0; j--)//���ұ߽����һ����������Ƿ�����߽��
                {
                    if (isLeftPoint(j, i))
                    {
                        imageLine.Exist_Left[i] = true;
                        imageLine.Point_Left[i] = j;
                        break;//ֻҪ�ҵ�һ����߽��Ͳ����ˣ���Ϊ����û�ж�
                    }
                }
            }
        }
        imageLine.Lost_Left = false;

        //�ڶ������------
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
    //2. ֻ�����ұ���
    else if (!imageLine.Lost_Left && imageLine.Lost_Right)
    {
        temp_rightLine_lost = true;
        //���߶��� ����һ������----------
        for (i = EFFECTIVE_ROW+6; i < MT9V03X_H; i++)//��������
        {
            if (imageLine.Exist_Left[i])//�ҵ���߽��
            {
                for (j = imageLine.Exist_Left[i] + 1; j < MT9V03X_W; j++)//����߽�ͬһ�п�ʼ���ұ߽��
                {
                    if (isRightPoint(j, i))
                    {
                        imageLine.Exist_Right[i] = true;
                        imageLine.Point_Right[i] = j;
                        break;//ֻҪ�ҵ�һ���ұ߽��Ͳ����ˣ���Ϊ����û�ж�
                    }
                }
            }
        }
        imageLine.Lost_Right = false;

        //�ڶ������------
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

    //ֱ�ߣ�������ж�-------
    if (!imageLine.Lost_Left && count1>30)//����û��������¼�����������
    {
        err1 = getLeastSquareMethodERROR(JudgeBasis_left[0], JudgeBasis_left[1], count1, k1, b1);
        //printf("L%f\r\n",err1);
        if (err1 > 0.38)
        {
            isLeftLineStraight = false;//�����Ƿ�Ϊֱ��
        }
        else
        {
            isLeftLineStraight = true;//�����Ƿ�Ϊֱ��
        }
    }
    if (!imageLine.Lost_Right && count2>30)//����û��������¼�����������
    {
        err2 = getLeastSquareMethodERROR(JudgeBasis_right[0], JudgeBasis_right[1], count2, k2, b2);
        //printf("R%f\r\n",err2);

        if (err2 > 0.38)
        {
            isRightLineStraight = false;//�����Ƿ�Ϊֱ��
        }
        else
        {
            isRightLineStraight = true;//�����Ƿ�Ϊֱ��
        }
    }
    if (imageLine.Lost_Left && imageLine.Lost_Right)//������ұ��߶�����
    {
        isLeftRightLineExist = false;
    }
    //������֮��߽�㶼��Ҫ��-----------------------
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
*Description: ��ʼ�β���
              ���������һ���ֱ��߿����������۸�������
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

    //------------------------������-------------------------------
    //��������ʼ��
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (imageLine.Exist_Left[i])
        {
            leftLine_startPoint = i;
            break;
        }
    }
    //������ʼ����1/6��ʱ(ͼ��ײ����������ߵĲ����е����),����
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
        if (count == 5)//��5���㼴�ɿ�ʼ����
        {
            leastSquareMethod(MendBasis_left[0], MendBasis_left[1], 5, &k_left, &b_left);

            //��ʼ����
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

    //------------------------������-------------------------------
    //��������ʼ��
    for (i = MT9V03X_H - 1; i >= 0; i--)
    {
        if (imageLine.Exist_Right[i])
        {
            rightLine_startPoint = i;
            break;
        }
    }
    //������ʼ����1/6��ʱ(ͼ��ײ����������ߵĲ����е����),����
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
        if (count == 5)//��5���㼴�ɿ�ʼ����
        {
            leastSquareMethod(MendBasis_right[0], MendBasis_right[1], 5, &k_right, &b_right);

            //��ʼ����
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
*Description: �����β���
              �Ź��񲹱߽���
*parameter: *
*Return: *
*****************************************************************/
void trackMend_endPart(void)
{
    bool leftIsAllRight = true; //�����Ƿ�һֱ����
    bool rightIsAllLeft = true;//�����Ƿ�һֱ����

    uint8_t leftTopPoint = 0;//��߽������Ч��
    uint8_t rightTopPoint = 0;//�б߽������Ч��

    uint8_t count = 0;

    int8_t i, j;//��Զ�Ĺ���i����j
    uint8_t tempPointer1;
    short tempPointer_Val1;
    uint8_t tempPointer2;
    short tempPointer_Val2;
    //�Ź�������---------------------------------
    //1. �ж������Ƿ�һֱ����
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
                        break;//��һ��i
                    }
                }
            }
            count++;
            leftTopPoint = i;//�������ҵ���߽��
        }
    }
    //2. ��������һֱ���ң���ʼ�˵�Ѱ��
    if (leftIsAllRight && (count > 15))
    {


        tempPointer1 = leftTopPoint;//�������ҵ���߽��
        tempPointer_Val1 = imageLine.Point_Left[tempPointer1];

        while (1)
        {
            //����
            if (isEdgePoint(tempPointer_Val1 + 1, tempPointer1 - 1))
            {
                tempPointer1 = tempPointer1 - 1;
                tempPointer_Val1 = tempPointer_Val1 + 1;
                imageLine.Exist_Left[tempPointer1] = true;
                imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
            }
            //��
            else if (isEdgePoint(tempPointer_Val1, tempPointer1 - 1))
            {
                tempPointer1 = tempPointer1 - 1;
                imageLine.Exist_Left[tempPointer1] = true;
                imageLine.Point_Left[tempPointer1] = tempPointer_Val1;
            }
            //��
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

    //�Ź�������---------------------------------
    count = 0;
    //1. �ж������Ƿ�һֱ����
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
                        break;//��һ��i
                    }
                }
            }
            count++;
            rightTopPoint = i;//����������ұ߽��
        }
    }
    //2. ��������һֱ���󣬿�ʼ�˵�Ѱ��
    if (rightIsAllLeft && (count > 15))
    {


        tempPointer2 = rightTopPoint;//����������ұ߽��
        tempPointer_Val2 = imageLine.Point_Right[tempPointer2];

        while (1)
        {
            //����
            if (isEdgePoint(tempPointer_Val2 - 1, tempPointer2 - 1))
            {
                tempPointer2 = tempPointer2 - 1;
                tempPointer_Val2 = tempPointer_Val2 - 1;
                imageLine.Exist_Right[tempPointer2] = true;
                imageLine.Point_Right[tempPointer2] = tempPointer_Val2;
            }
            //��
            else if (isEdgePoint(tempPointer_Val2, tempPointer2 - 1))
            {
                tempPointer2 = tempPointer2 - 1;
                imageLine.Exist_Right[tempPointer2] = true;
                imageLine.Point_Right[tempPointer2] = tempPointer_Val2;
            }
            //��
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
*Description: �������� �������
              ���Ҷ�û�� ֱ�Ӽ�������
*parameter: *
*Return: *
*****************************************************************/
void trackMend_HalfWidth(void)
{
    int8_t i;
    float err = 0, aveErr = 0;
    uint8_t count = 0;
    uint8_t centerCompensation = 0;

    //(һ) ���߶���(û����)------------------------------------
    if (imageLine.Lost_Left && imageLine.Lost_Right)
    {
        imageLine.Lost_Center = true;
    }

    //(��) ֻ�������
    else if (imageLine.Lost_Left)
    {
        imageLine.Lost_Center = false;

        //1. �������߲���
        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Right[i])
            {
                err += (((MT9V03X_W) / 2 + roadK * i / 2 + roadB / 2) - imageLine.Point_Right[i]);
                count++;
            }
        }

        //2. ����ƽ�����
        aveErr = (float)(err / count);
        //ave_err_max = MAX(aveErr, ave_err_max);//���ڵ���

        //3. �����ұ��߲�����(·��+���߲���)
        if (count >= 5 && aveErr > 0)//�����㹻��ȷ���ұ߽���������б
            centerCompensation = LIMIT2MIN(aveErr, SingleLineLeanAveERR_MAX) / SingleLineLeanAveERR_MAX * SingleLineLeanK / 2;//��������

        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Right[i])
            {
                //�������߲��޷�
                imageLine.Exist_Center[i] = true;
                //amplitudeLIMIT(i, imageLine.Point_Right[i] - centerCompensation - roadK * i / 2 - roadB / 2 - (10 * flag_forkRoad_find / 2) - flag_more_SingleLineLeanK * 30);
                amplitudeLIMIT(i, imageLine.Point_Right[i] - centerCompensation - roadK * i / 2 - roadB / 2);
            }
        }

        if (flag_forkRoad_find == 1)
        {
            //��������²���������ͻ���޷�
            repairRemainLine();//����С���˷��޸�δ֪������
        }
        else
        {
            limitCenter();//�Եó������߽���ͻ���޷�
            //repairRemainLine();//����С���˷��޸�δ֪������
        }

    }

    //(��) ֻ���ұ���
    else if (imageLine.Lost_Right)
    {
        imageLine.Lost_Center = false;

        //1. �������߲���
        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Left[i])
            {
                err += (imageLine.Point_Left[i] - ((MT9V03X_W) / 2 - roadK * i / 2 - roadB / 2));
                count++;
            }
        }

        //2. ����ƽ�����
        aveErr = (float)(err / count);
        //ave_err_max = MAX(aveErr, ave_err_max);//���ڵ���

        //3. ��������߲�����(·��+���߲���)
        if (count >= 5 && aveErr > 0)//�����㹻��ȷ���ұ߽���������б
            centerCompensation = LIMIT2MIN(aveErr, SingleLineLeanAveERR_MAX) / SingleLineLeanAveERR_MAX * SingleLineLeanK / 2;//��������

        for (i = MT9V03X_H - 1; i >= 0; i--)
        {
            if (imageLine.Exist_Left[i])
            {
                //�������߲��޷�
                imageLine.Exist_Center[i] = true;
                //amplitudeLIMIT(i, imageLine.Point_Left[i] + centerCompensation + roadK * i / 2 + roadB / 2 + (10 * flag_forkRoad_find / 2) + flag_more_SingleLineLeanK * 15);
                amplitudeLIMIT(i, imageLine.Point_Left[i] + centerCompensation + roadK * i / 2 + roadB / 2);
            }
        }

        if (flag_forkRoad_find == 1)
        {
            //��������²���������ͻ���޷�
            repairRemainLine();//����С���˷��޸�δ֪������
        }
        else
        {
            //limitCenter();//�Եó������߽���ͻ���޷�
            repairRemainLine();//����С���˷��޸�δ֪������
        }
        limitCenter();//�Եó������߽���ͻ���޷�
        //repairRemainLine();//����С���˷��޸�δ֪������

    }

    //(��)���߶�û�ж���
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
                    //�������߲��޷�
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
*Description: ����С���˷��޸�δ֪������
              �����ǰ�������������ͷ�ľ�����˵�ģ�������ͼ��
              Ҳ����˵��ͼ����·�����Ϊǰ(��ʼ)
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
    short mediumLine1[2][MT9V03X_H], mediumLine2[2][MT9V03X_H];//��㵽�յ�1���߶Σ��յ�1���յ�2���߶�

    uint8_t count1, count2;

    float k1, k2, b1, b2;

    //(һ) ���ǰ�沿��
    //1. ���ҵ�������ĵ�һ����Ч�е�-----------------------
    for (i = MT9V03X_H - 1; i >= 0; i--)//��������
    {
        if (imageLine.Exist_Center[i])
        {
            mediumLineStart = i;
            break;
        }
    }

    //2. ���ȱ��----------------------------------------
    for (i = mediumLineStart - 1; i >= 0; i--)
    {
        if (!imageLine.Exist_Center[i])//��Ч����ʼ�е����һ���е�û��
        {
            x1 = i + 1;
            y1 = imageLine.Point_Center[x1];//��ʼ�е����һ�е�

            for (j = i - 1; j >= 0; j--)//���ϼ�������Ч�е�
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

    //3. ���ͼ���·�������-----------------------------------
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

    //(��)��Ϻ��沿��-----------------------------------
    //1. ���ҵ�ͼ�����������Ч�е�(������ֹ��)
    for (i = 0; i < MT9V03X_H; i++)
    {
        if (imageLine.Exist_Center[i])
        {
            mediumLineEnd = i;
            break;
        }
    }
    //2. ���ͼ���Ϸ�������
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
*Description: �Եó������߽��и�ֵ�޷�
              ���е㸳ֵ������Ϊ����������Ч
*parameter: i ���ߵ�������
            amp ���ߵ�������
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
*Description: �Եó������߽���ͻ���޷�
*parameter: *
*Return: *
*****************************************************************/
void limitCenter(void)
{
    for (int i = MT9V03X_H - 2; i >= 1; i--)//��������
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
*Description: ����У��
*parameter: *
*Return: *
*****************************************************************/
void mediumLineCheck(void)
{
    uint8_t i;

    if (!imageLine.Lost_Center)//��������û��
    {
        //(һ) �Ҷ���
        if (imageLine.Lost_Right && !imageLine.Lost_Left)
        {
            //ͼ���ϰ벿������δ������ֵ������δԽ���м�������Ч
            short leftMAX = 0;//��ʾ��߽������������쵽������
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

        //(��) ����
        else if (imageLine.Lost_Left)
        {
            //ͼ���ϰ벿������δ������ֵ������δԽ���м�������Ч
            short rightMIN = MT9V03X_W;//��ʾ�ұ߽������������쵽������
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

        //(��) ������
        else
        {
            uint8_t len_temp;
            uint8_t len_basis;
            uint8_t lostCenter_cnt = 0;

            for (i = MT9V03X_H / 2; i < MT9V03X_H; i++)//��ͼ���м俪ʼ���¼��
            {
                //����������ұߵ����
                if (imageLine.Exist_Left[i] && imageLine.Exist_Right[i])
                {
                    len_temp = imageLine.Point_Right[i] - imageLine.Point_Left[i];
                    len_basis = roadK * i + roadB;

                    //����ʵ��·��Ӧȫ�������len_basis - 10
                    //�粻����, ��Ч���߲�����ѭ��
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
*Description: ���׺ڰ��Ų�
*parameter: row����
*           half��0���ߣ�1�Ұ�ߣ�2ȫͼ
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
volatile uint8_t AIM_LINE = AIM_LINE_SET;//Ŀ���У����������е���Ϊ�Ĳο��У�

/*****************************************************************
*Function: updateMediumLine(*)
*Description: �������ߴ��
*parameter: *
*Return: *
*****************************************************************/
void updateMediumLine(void)
{
    bool rst;
    static uint8_t lostTime = 0;



    //1. Ѱ������
    //ImageProcessInit();//������ʼ��
    image_pre_processing();//ͼ��Ԥ����
    Get_White_Num(0);//��ȡÿһ�еİ׵����
    trackDFS();//DFS��ȱ���
    if(!flag_isRight_ring && !flag_isLeft_ring)
        forkRoad_find();//����·���ж�
    if(flag_garage_turn==2)
        zebra_cross_detect();
    //forkRoad_turnPoint_prefind();//////////////////////////////////////б������
    //forkRoad_turnPoint_find();//////////////////////////////////////
    left_right_Limit();

    lineChangeLimit();//�߽���ͻ��
    lost_mend();
    doFilter();//�Կ��ܵĴ������й���

    doMend();//���߳���
    if (flag_forkRoad_find == 0 && flag_isLeft_ring == 0 && flag_isRight_ring == 0)
        mediumLineCheck();//����У�����
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
    //2. �������ߴ��
    rst = MediumLineCal(camERR.cam_finalCenterERR);

    //3. ���ߴ��
    if (rst)//����ͷ���
    {
        lostTime = 0;
        //flag.turnWAY = 0;
    }
    else if (!rst && (lostTime <= 2))//����ͷ���
    {
        lostTime++;
    }
}

/*****************************************************************
*Function: MediumLineCal(*)
*Description: �������ߴ��
              �������ļ��㻹Ҫ���ԣ�����
*parameter: *
*Return: *
*****************************************************************/
bool MediumLineCal(volatile short * final)
{
    uint8_t i;
    static float lastCenter = 47;//94*60  ����������47

    AIM_LINE = AIM_LINE_SET;

    if (imageLine.Exist_Center[AIM_LINE] == false && !imageLine.Lost_Center)
    {
        //��������д����߲����ڣ������Ҹ���
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
            //�����������
            for (i = 0; i < 2; i++)
                *(final + 2 - i) = *(final + 1 - i);

            //���������������
            *(final) = (imageLine.Point_Center[AIM_LINE] - center);

            lastCenter = imageLine.Point_Center[AIM_LINE];
        }
    }
    return 1;
}







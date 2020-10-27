#include "machine.h"
#include "input.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

typedef enum MachineState
{
    MAIN = 0,
    CLOCK_IN,
    CLOCK_OUT,
    SHOW_TIME,

} MachineState;

static int g_numOfStaffs = 0;         //员工数量
static AttendRecord *g_AttendData;    //员工打卡数据

static MachineState g_state = MAIN;   //机器状态
static int g_openTime_sec = 7 * 3600; //每天上班营业时间, 单位秒
static int g_workingHours = 9;        //上班时长

static time_t g_currentT = 0;         //当前时间秒
static SysTime g_stime = {0};         //当前时间

static int sysInit(void);
static int dealWith(const char input[], int length);
static int funcMain(const char input[], int length);
static int funcClockIn(const char input[], int length);
static int funcClockOut(const char input[], int length);
static int refreshInterface(MachineState newState, int delay);
static int clockInRecord(AttendRecord *staff, SysTime stime);
static int clockOutRecord(AttendRecord *staff, SysTime stime);
static int getCheckCode(const char number[], int length, char checkCode[]);
static int printReport(FILE *fp, AttendRecord *staff, int days);
static void *timeThread(void*); 


void clockin_machine_start(void)
{
    char inputStr[BUFF_SIZE] = {0};
    int length = BUFF_SIZE - 1;
    int getLen = 0;
    pthread_t threadHandle = 0;
    printf("clockin machine start !\r\n");
    sysInit(); //打卡机初始化
    if (pthread_create(&threadHandle, NULL, timeThread, NULL)) 
    {
        fprintf(stderr, "create time thread failed ! \r\n");
        exit(-1);
    }
    while (1)
    {
        getLen = getInputStr(inputStr, length);
        if (getLen > 0)
        {
            if (inputStr[0] == 'q') //退出
            {
                FILE *fp = fopen(REPORT_FILE, "w+t");
                printf("writing file : %s\r\n", REPORT_FILE);
                genWeeklyReport(fp);
                fclose(fp);
                break;
            }
            dealWith(inputStr, length); //处理输入
        }
    }
    printf("attendance machine has done !\r\n");
}

int sysInit(void)
{
    int i = 0;
    FILE *fp = fopen(STAFFS_FILE, "rt"); //保存员工信息的文件
    char str[1000];
    if (fp == NULL)
    {
        fprintf(stderr, "%s is not exist ！\r\n", STAFFS_FILE);
        exit(-1);
    }
    g_openTime_sec = 7 * 3600; 
    g_workingHours = 9;
    g_numOfStaffs = 0;
    //查看文件信息数量
    printf("reading file %s\r\n", STAFFS_FILE);
    while(fgets(str, 1000, fp) != NULL)
    {
        g_numOfStaffs++; //存在一行
    }
    printf("there are %d staffs data .\r\n",g_numOfStaffs);
    g_AttendData = (AttendRecord *) malloc(g_numOfStaffs * sizeof(AttendRecord));
    rewind(fp);
    //读取文件内所有编号
    for (i = 0; i < g_numOfStaffs; i++)
    {
        int number = 0;
        int readArg = 0;
        readArg = fscanf(fp, "%d:%*[^\n]\n", &number); //获取文件一行的编号
        if (readArg < 1)
        {
            fprintf(stderr, "reading file error: in file:%s line:%d\r\n",__FILE__, __LINE__); //读取出错
        }
        sprintf(g_AttendData[i].number,"%d",number);
        memset(g_AttendData[i].record, UNRECORD, sizeof(g_AttendData[i].record)); 
    }
    printf("System startup ! \r\n");
    refreshInterface(MAIN, DELAY_OK);
    fclose(fp);
    return 0;
}


static void *timeThread(void *arg)
{
    time_t curT = -1;
    size_t baseT = time(NULL);
    while (1)
    {
        g_currentT = TIME_RATIO * (time(NULL) - baseT);
        convertTime(g_currentT, &g_stime);
        if (curT > 5 * 24 * 3600)
        {
            FILE *fp = fopen(REPORT_FILE, "w+t");
            printf("writing file : %s\r\n", REPORT_FILE);
            genWeeklyReport(fp);
            fclose(fp);
        }
    }
    pthread_exit(NULL);

}

int convertTime(time_t second, SysTime *sysTime)
{
    sysTime->day = (second / (24 * 3600)) % 7;
    sysTime->hour = (second / 3600) % 24;
    sysTime->minute = (second / 60) % 60;
    sysTime->second = second % 60;
    return 0;
}
int getStaffNum()
{
    return g_numOfStaffs;
}

int dealWith(const char input[], int length)
{
    switch (g_state)
    {
    case MAIN:
        funcMain(input, length);
        break;
    case CLOCK_IN:
        funcClockIn(input, length);
        break;
    case CLOCK_OUT:
        funcClockOut(input, length);
        break;
    default:
        break;
    }
    return 0;
}

int genWeeklyReport(FILE *fp)
{
    
    int i = 0;
    fprintf(fp, "\r\n");
    fprintf(fp, "|----------------Weekly  report ----------------|\r\n");
    for (i = 0; i < g_numOfStaffs; i++)
    {
        printReport(fp, &g_AttendData[i],5);
    }
    fprintf(fp, "|------------------- E N D ---------------------|\r\n");
    return 0;
}


static int funcMain(const char input[], int length)
{
    switch (input[0])
    {
    case '1':
        refreshInterface(CLOCK_IN, 0);
        break;
    case '2':
        refreshInterface(CLOCK_OUT, 0);
        break;
    case '3':
        refreshInterface(SHOW_TIME, 0);
        break;
    case '4':
        genWeeklyReport(stdout);
        refreshInterface(MAIN, DELAY_OK);
        break;
    case 'q':
        break;
    default:
        printf("input error ! \r\n");
        refreshInterface(MAIN, DELAY_FAIL);
        return -1;
    }
    return 0;
}


static int funcClockIn(const char input[], int length)
{
    static int s_step = 0;
    static AttendRecord *s_staff = NULL;
    char checkCode[DIGITS];

    switch (s_step)
    {
    case 0:
        s_staff = findStaff(input);
        if (s_staff == NULL)
        {
            //找不到工号
            printf("corresponding data not found !\r\n");
            refreshInterface(MAIN, DELAY_FAIL);
            return -1;
        }
        if (s_staff->record[g_stime.day][0] != UNRECORD)
        {
            //已经打过卡
            printf("you've already clocked in !\r\n");
            refreshInterface(MAIN, DELAY_FAIL);
            return -1;
        }

        printf("hello : %s \r\n", input); //欢迎
        s_step = 1;                       //进入验证码状态
        printf("please enter your check code :\r\n");
        break;
    case 1:
        getCheckCode(s_staff->number, DIGITS, checkCode); //获取检验码
        if (equalChar(input, checkCode, DIGITS))
        {
            clockInRecord(s_staff, g_stime); //打卡成功
            s_staff = NULL;
            s_step = 0;                         //返回等待工号状态
            refreshInterface(MAIN, DELAY_OK); //显示主页面
        }
        else
        {
            printf("verification failed. \r\n");
            printf("try again ? (yes/no) :\r\n");
            s_step = 2; //进入重试答复状态
        }
        break;
    case 2:
        if (equalChar(input, "yes", 3))
        {
            printf("please enter your check code again:\r\n");
            s_step = 1; //进入验证码状态
            break;
        }
        if (equalChar(input, "no", 2))
        {
            s_staff = NULL;
            s_step = 0;                    //返回等待工号状态
            refreshInterface(MAIN, 200); //显示主页面
            break;
        }
        printf("try again ? (yes/no) :\r\n");
        break;
    default:
        printf("system error ! \r\n");
        refreshInterface(MAIN, DELAY_FAIL);
        break;
    }
    return 0;
}

static int funcClockOut(const char input[], int length)
{
    AttendRecord *staff = findStaff(input);
    if (staff == NULL)
    {
        //找不到员工
        printf("corresponding data not found !\r\n");
        refreshInterface(MAIN, DELAY_FAIL);
        return -1;
    }
    if (staff->record[g_stime.day][1] != UNRECORD)
    {
        //已经打过卡
        printf("you've already clocked out !\r\n");
        refreshInterface(MAIN, DELAY_FAIL);
        return -1;
    }
    if (staff->record[g_stime.day][0] == UNRECORD && g_stime.hour < 12)
    {
        //应该是打上班卡
        printf("you should clock in firstly !\r\n");
        refreshInterface(CLOCK_IN, DELAY_FAIL); //跳转到上班打卡界面
        return -1;
    }
    clockOutRecord(staff, g_stime);     //打卡成功
    refreshInterface(MAIN, DELAY_OK); //显示主页面
    return 0;
}

static int clockInRecord(AttendRecord *staff, SysTime stime)
{
    int ct_sec = stime.hour * 3600 + stime.minute * 60 + stime.second; //当前时间 单位秒
    char strTime[1024] = {0};
    staff->record[stime.day][0] = ct_sec;
    //迟到检测
    if (ct_sec > g_openTime_sec)
    {
        //已经过了正常上班时间
        if (ct_sec - g_openTime_sec > 2 * 3600 || stime.day == 0 || staff->workingTime[stime.day - 1] < 3 * 3600)
        {
            staff->abnormal[stime.day][0] = 1; //记录异常
            printf("you are late !\r\n");
        }
    }
    sprintTime(strTime, &stime);
    printf("%s \r\n", strTime);
    printf("clock in successfully. \r\n");
    return 0;
}

/**
 * 记录下班打卡
 * 
*/
static int clockOutRecord(AttendRecord *staff, SysTime stime)
{
    int ct_sec = stime.hour * 3600 + stime.minute * 60 + stime.second; //当前时间 单位秒
    staff->record[stime.day][1] = ct_sec;
    //早退检测
    if (ct_sec < g_openTime_sec + g_workingHours * 3600)
    {
        //还未到正常下班时间
        staff->abnormal[stime.day][1] = 1; //记录异常
        printf("you left early !\r\n");
    }
    //计算工作时长
    if (staff->record[stime.day][0] != UNRECORD)
    {
        //打过上班卡
        staff->workingTime[stime.day] = ct_sec - staff->record[stime.day][0];
    }
    printf("you worked for %d hours today\r\n", staff->workingTime[stime.day] / 3600);
    printf("clock out successfully. \r\n");
    return 0;
}
/**
 * 过一段时间后更新状态并 打印界面
 */
static int refreshInterface(MachineState newState, int delay)
{
    char str[1024] = {0};
    sleep_ms(delay);
    g_state = newState;
    printf("\r\n");
    switch (newState)
    {
    case MAIN:
        printf("|---------------------------------| \r\n");
        printf("|-------Attendance machine--------| \r\n");
        printf("|---------------------------------| \r\n");
        printf("please enter your choice \r\n");
        printf("1: clock in\r\n");
        printf("2: clock out\r\n");
        printf("3: show current time\r\n");
        printf("4: show weekly report\r\n");
        printf("q: quit\r\n");
        break;
    case CLOCK_IN:
        printf("|---------------------------------| \r\n");
        printf("|-----------Clock  in-------------| \r\n");
        printf("|---------------------------------| \r\n");
        printf("please enter your number : \r\n");
        break;
    case CLOCK_OUT:
        printf("|---------------------------------| \r\n");
        printf("|-----------Clock  out------------| \r\n");
        printf("|---------------------------------| \r\n");
        printf("please enter your number : \r\n");
        break;
    case SHOW_TIME:
        printf("|---------------------------------| \r\n");
        printf("|---------Current  time-----------| \r\n");
        printf("|---------------------------------| \r\n");
        sprintTime(str, &g_stime);
        printf("%s", str);
        refreshInterface(MAIN, 1000);//return to MAIN page
    default:
        break;
    }
    return 0;
}

int sprintTime(char *str, const SysTime *st)
{
    sprintf(str, "%s  %02d : %02d : %02d  \r\n", getDayStr(st->day), st->hour, st->minute, st->second);
    return 0;
}
static int printReport(FILE *fp, AttendRecord *staff,int days)
{
    int totalHours = 0; //总工作时长 小时
    int absence = 0;    //缺卡 次数
    int late = 0;       //迟到次数
    int early = 0;      //早退次数
    int i = 0;
    for (i = 0; i < days; i++)
    {
        totalHours += staff->workingTime[i];
        //上班打卡有异常
        if (staff->record[i][0] == UNRECORD)
        {
            absence++; //缺卡
        }
        else if (staff->abnormal[i][0])
        {
            late++; //迟到
        }

        //下班打卡有异常
        if (staff->record[i][1] == UNRECORD)
        {
            absence++; //缺卡
        }
        else if (staff->abnormal[i][1])
        {
            early++; //早退
        }
    }
    totalHours /= 3600; //换算成小时

    fprintf(fp, "Number : %s ", staff->number);
    fprintf(fp, "totalHours : %d\t", totalHours);
    fprintf(fp, "absence : %d\t", absence);
    fprintf(fp, "come late : %d\t", late);
    fprintf(fp, "left early : %d\t", early);
    fprintf(fp, "\r\n");
    return 0;
}
char *getDayStr(int day)
{
    switch (day)
    {
    case 0:
        return "Monday";
        break;
    case 1:
        return "Tuesday";
        break;
    case 2:
        return "Wednesday";
        break;
    case 3:
        return "Thursday";
        break;
    case 4:
        return "Friday";
        break;
    case 5:
        return "Saturday";
        break;
    case 6:
        return "Sunday";
        break;
    default:
        return "--";
        break;
    }
    return NULL;
}

AttendRecord *findStaff(const char number[])
{
    int i = 0;
    for (i = 0; i < g_numOfStaffs; i++)
    {
        //判断是不是这位员工
        if (equalChar(g_AttendData[i].number, number, DIGITS))
        {
            //找到了
            return &g_AttendData[i];
        }
    }
    return NULL;
}

int equalChar(const char str1[], const char str2[], int length)
{
    int equal = 0;
    int i = 0;
    for (i = 0; i < length; i++)
    {
        if (str1[i] == str2[i])
        {
            equal++;
        }
        else
        {
            break;
        }
    }
    if (equal == length)
    {
        //找到了
        return 1;
    }
    return 0;
}

static int getCheckCode(const char number[], int length, char checkCode[])
{
    int posNum = 0;
    int invNum = 0;
    int result = 0;
    int i = 0;
    int temp = 1;
    for (i = 0; i < length; i++)
    {
        int num = number[i] - '0';//对应位的数字
        posNum = posNum * 10 + num;//十进制左移 再补充最低位
        invNum += num * temp;
        temp *= 10;
    }
    invNum /= 10;//十进制右移,去掉最低位的1
    result = posNum + invNum;
    //填充 checkCode
    for (i = 0; i < length; i++)
    {
        checkCode[length - i - 1] = result % 10 + '0';
        result /= 10;
    }
    checkCode[length] = '\0';
    return 0;
}

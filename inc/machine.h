/*
 * @Author: your name
 * @Date: 2020-09-29 06:04:02
 * @LastEditTime: 2020-10-18 02:54:07
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Project/AttendanceMachine/inc/machine.h
 */
#ifndef MACHINE_H
#define MACHINE_H
#include <time.h>
#include <stdio.h>
#if _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

#define DIGITS  6    //编号位数 
#define DELAY_OK  600   //提示操作成功后，显示界面延时
#define DELAY_FAIL  1000 //提示操作失败后，显示界面延时
#define TIME_RATIO 600 //6s = 1h
#define REPORT_FILE "./data/weeklyReport.txt"
#define STAFFS_FILE "./data/staffs.txt"
#define UNRECORD -1 //没有记录时的打卡默认设置

typedef struct SysTime
{
    int day;//天
    int hour;//小时
    int minute;//分钟
    int second;//秒
}SysTime;

typedef struct AttendRecord
{
    char number[DIGITS + 1]; //编号
    int record[7][2];//打卡时间记录，当天第几秒
    int abnormal[7][2];//异常记录
    int workingTime[7];//每天工作时间 秒
}AttendRecord;

#if _WIN32
#define sleep_ms(ms) Sleep(ms)
#elif __linux__
#define sleep_ms(ms) usleep(1000 * ms)
#endif

void clockin_machine_start(void);
int getStaffNum();
int genWeeklyReport(FILE *fp);
int sprintTime(char *str, const SysTime *st);
int equalChar(const char str1[], const char str2[], int length);
int convertTime(time_t second, SysTime *sysTime);
char *getDayStr(int day);
AttendRecord *findStaff(const char number[]);

#endif
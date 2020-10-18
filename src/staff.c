/*
 * @Author: your name
 * @Date: 2020-10-12 07:42:10
 * @LastEditTime: 2020-10-18 03:27:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /StaffManagement/src/staff.c
 */
#include <staff.h>
#include <string.h>
#include <stdlib.h>
#define STR_BUFF_SIZE 1000

int start_staff_manager(void)
{
    FILE *fpStaffs = fopen(STAFFS_FILE, "rt"); //保存员工信息的文件
    FILE *fpList = fopen(LIST_FILE, "w+t"); //保存奖惩名单文件
    char str[STR_BUFF_SIZE];
    int numOfStaffs;
    StaffData *allData;
    int i = 0;
    if (fpStaffs == NULL)
    {
        fprintf(stderr, "%s is not exist ！\r\n", STAFFS_FILE);
        exit(-1);
    }
    clockin_machine_start();     //运行打卡机
    numOfStaffs = getStaffNum(); //员工人数
    allData = (StaffData *)malloc(numOfStaffs * sizeof(StaffData));

    for (i = 0; i < numOfStaffs; i++)
    {
        int readArg = 0;
        fgets(str, STR_BUFF_SIZE, fpStaffs);
        readArg = sscanf(str,
                         "%[^:]:%[^:]:%d:%c:%[^:]:%s\n",
                         allData[i].number,
                         allData[i].name,
                         &allData[i].age,
                         &allData[i].gender,
                         allData[i].phoneNumber,
                         allData[i].email);
        if (readArg < 6)
        {
            fprintf(stderr, "reading file error: in file:%s line:%d\r\n",__FILE__, __LINE__); //读取出错
        }
        allData[i].record = findStaff(allData[i].number);
    }
    printf("writing file : %s\r\n",LIST_FILE);
    genNameList(fpList, allData, numOfStaffs);//打印名单
    free(allData);
    fclose(fpList);
    fclose(fpStaffs);
    return 0;
}

int genNameList(FILE *fp, StaffData *data, int staffNUm)
{
    int i = 0;
    int workHours[staffNUm]; //记录良好员工的工作时间
    int best[3] = {-1,-1,-1};//记录最长时间的前三名的下标
    memset(workHours, 0, staffNUm * sizeof(int));
    fprintf(fp, "\r\n");
    fprintf(fp, "|--------------- Punishment List ---------------|\r\n");
    for (i = 0; i < staffNUm; i++)
    {
        char str[1024] = {0};
        char temp[1024] = {0};
        int totalHours = 0; //总工作时长 小时
        int absence = 0;    //缺卡 次数
        int late = 0;       //迟到次数
        int early = 0;      //早退次数
        int j = 0;
        AttendRecord *staff = data[i].record;
        sprintf(str, " \r\nnumber: %s  name: %s ", data[i].number, data[i].name);
        
        for (j = 0; j < 5; j++)
        {
            totalHours += staff->workingTime[j];
            if (staff->record[j][0] == UNRECORD || staff->record[j][1] == UNRECORD)
            {
                if (absence == 0)
                {
                    strcat(str, "\r\nabsence: ");
                }
                absence++; //缺卡
                sprintf(temp, " %s ", getDayStr(j));
                strcat(str, temp);
            }
        }
        for (j = 0; j < 5; j++)
        {
            if (staff->abnormal[j][0])
            {
                if (late == 0)
                {
                    strcat(str, "\r\ncome late: ");
                }
                late++; //迟到
                sprintf(temp, " %s ", getDayStr(j));
                strcat(str, temp);
            }
        }
        for (j = 0; j < 5; j++)
        {
            if (staff->abnormal[j][1])
            {
                if (late == 0)
                {
                    strcat(str, "\r\nleft early: ");
                }
                early++; //早退
                sprintf(temp, " %s ", getDayStr(j));
                strcat(str, temp);
            }

        }
        totalHours /= 3600; //换算成小时
        if (absence == 0 && late == 0 && early == 0)
        {
            workHours[i] = totalHours;
        }
        else
        {
            workHours[i] = 0;
            fprintf(fp, "%s", str);
        }
        fprintf(fp, "\r\n");
    }
    fprintf(fp, "|------------------- E N D ---------------------|\r\n");

    fprintf(fp, "\r\n");
    fprintf(fp, "|------------------ Award List -----------------|\r\n");
    for (i = 0; i < staffNUm; i++)
    {
        int j = 0;
        int temp1 = i;
        if (workHours[i] == 0) continue;
        
        for (j = 0; j < sizeof(best)/sizeof(int); j++)
        {
            int temp2 = 0;
            if (best[j] == -1 ||workHours[temp1] >= workHours[best[j]])
            {
                temp2 = best[j];
                best[j] = temp1;
                temp1 = temp2;
            }
        }
    }
    for (i = 0; i < sizeof(best)/sizeof(int); i++)
    {
        if (best[i] == -1) break;
        fprintf(fp, "number: %s  name: %s  workHourse: %d\r\n", data[best[i]].number, data[best[i]].name, workHours[best[i]]);
    }
    fprintf(fp, "|------------------- E N D ---------------------|\r\n");
    return 0;
}

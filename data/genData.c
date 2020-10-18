/*
 * @Author: your name
 * @Date: 2020-10-12 09:01:46
 * @LastEditTime: 2020-10-14 10:07:29
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Project/StaffManagement/data/init.c
 */


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//工号，姓名，年龄，性别，手机号，邮箱地址
const char *randName(int length);

int main(int argc, char const *argv[])
{
    int number = 110086;
    FILE *fp = fopen("staffs.txt", "w+t");
    srand(time(NULL));
    for (int i = 0; i < 20; i++)
    {
        fprintf(fp, "%d:", number + i);
        fprintf(fp, "%s:", randName(6));
        fprintf(fp, "%d:", rand() % 20 + 18);
        fprintf(fp, "%c:", (rand() % 2 == 0)?'M':'F');
        fprintf(fp, "%ld:", 13718500000L +  rand() % 1000000);
        fprintf(fp, "%d@kkb.com", number + i);
        fprintf(fp, "\n");
    }

    return 0;
}

const char *randName(int length)
{
    static char name[100];
    name[0] = '\0';
    for (int i = 0; i < length; i++)
    {
        name[i] = rand() % 26 + 'a';
    }
    name[length] = '\0';
    return name;
}
/*
 * @Author: your name
 * @Date: 2020-10-12 07:42:21
 * @LastEditTime: 2020-10-18 02:44:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Project/StaffManagement/inc/manage.h
 */
#ifndef STAFF_H
#define STAFF_H
#include "machine.h"
#define LIST_FILE "./data/nameList.txt"

typedef struct staffData{
    char number[DIGITS+1];
    char name[10];
    int  age;
    char gender;
    char phoneNumber[12];
    char email[20];
    AttendRecord *record;
} StaffData;

int start_staff_manager(void);
int genNameList(FILE *fp, StaffData *data, int staffNUm);
#endif
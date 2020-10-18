/*
 * @Author: your name
 * @Date: 2020-09-29 06:04:02
 * @LastEditTime: 2020-10-18 02:40:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Project/AttendanceMachine/src/input.c
 */
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stddef.h>

static char myGetChar(void); 

int getInputStr(char inputStr[], int length)
{
    static char s_inputBuff[BUFF_SIZE] = {0}; //输入缓冲区
    static int s_inputLen = 0;                //已获得输入的长度
    int getLen = 0;                           //实际提交的长度
    char inputChar = -1;                      //当前输入字符

    if (length > BUFF_SIZE - 1)
    {
        memset(inputStr, 0, length);
        printf("\r\n  system error ! \r\n");
        exit(-1); //严重错误
    } 
    inputChar = myGetChar();
    switch (inputChar)
    {
    case -1:
        break;
    case '\b':
    case 127:
        if (s_inputLen > 0)
        {
            s_inputBuff[--s_inputLen] = '\0';
        }
        break;
    case ' ':
    case '\r':
    case '\n':
        getLen = s_inputLen; //提前获取
        s_inputBuff[getLen] = '\0';
        break;
    default:
        s_inputBuff[s_inputLen++] = inputChar;
        break;
    }
    if (s_inputLen >= length)
    {
        getLen = length; //获得了指定长度
        s_inputBuff[getLen] = '\0';
    }
    if (getLen > 0)
    {
        memcpy(inputStr, s_inputBuff, getLen + 1); //复制需要获取的长度和‘\0’
        memset(s_inputBuff, 0, BUFF_SIZE);
        s_inputLen = 0;
        return getLen;
    }
    return 0;
}

static char myGetChar(void)
{
    char ch = -1;
#if _WIN32
    while (kbhit()) //检测缓冲区中是否有数据
    {
        ch = getch(); 
        putchar(ch);
        fflush(stdout);  //本次输出结束后立即清空缓冲区
    }
#elif __linux__

    fd_set rfds;
    struct timeval tv;
    system(STTY_US TTY_PATH);
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    while (select(1, &rfds, NULL, NULL, &tv) > 0)
    {
        ch = getchar();
        putchar(ch);
        fflush(stdout);  //本次输出结束后立即清空缓冲区
    }
#endif
    return ch;
}


/*
 * @Author: your name
 * @Date: 2020-09-29 06:04:02
 * @LastEditTime: 2020-10-03 22:36:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Project/AttendanceMachine/input.h
 */
#ifndef INPUT_H
#define INPUT_H

#if _WIN32
#include <conio.h>
#elif __linux__
#include <termio.h>
#define TTY_PATH            "/dev/tty "
#define STTY_US             "stty raw -echo -F"
#define STTY_DEF            "stty -raw echo -F"
#endif



#define BUFF_SIZE 32
int getInputStr(char inputStr[], int length);

#endif
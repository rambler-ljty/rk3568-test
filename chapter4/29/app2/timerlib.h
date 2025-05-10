/*
 * 定时器库头文件
 * 定义了定时器操作相关的函数和命令
 */

#ifndef _TIMELIB_H_
#define _TIMELIB_H_
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>

/* 定义ioctl命令，需要与内核模块中的定义保持一致 */
#define TIMER_OPEN _IO('L',0)     // 打开定时器
#define TIMER_CLOSE _IO('L',1)    // 关闭定时器
#define TIMER_SET _IOW('L',2,int) // 设置定时器时间

/* 函数声明 */
int dev_open();           // 打开设备文件
int timer_open(int fd);   // 启动定时器
int timer_close(int fd);  // 关闭定时器
int timer_set(int fd,int arg); // 设置定时器时间

#endif

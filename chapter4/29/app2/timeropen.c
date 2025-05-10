/*
 * 定时器打开函数实现
 * 通过ioctl命令启动内核定时器
 */

#include<stdio.h>
#include "timerlib.h"

/* 启动定时器函数 */
int timer_open(int fd)
{
	int ret;
	// 发送TIMER_OPEN命令启动定时器
	ret=ioctl(fd,TIMER_OPEN);
	if(ret<0){
		printf("ioctl open error \n");
		return -1;
	}
	return ret;
}

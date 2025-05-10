/*
 * 定时器设置函数实现
 * 通过ioctl命令设置内核定时器的时间
 */

#include<stdio.h>
#include "timerlib.h"

/* 设置定时器时间函数 */
int timer_set(int fd,int arg)
{
	int ret;
	// 发送TIMER_SET命令设置定时器时间
	ret=ioctl(fd,TIMER_SET,arg);
	if(ret<0){
		printf("ioctl error \n");
		return -1;
	}
	return ret;
}

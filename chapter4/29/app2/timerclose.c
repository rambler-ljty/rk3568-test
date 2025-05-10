/*
 * 定时器关闭函数实现
 * 通过ioctl命令停止内核定时器
 */

#include<stdio.h>
#include "timerlib.h"

/* 关闭定时器函数 */
int timer_close(int fd)
{
	int ret;
	// 发送TIMER_CLOSE命令停止定时器
	ret=ioctl(fd,TIMER_CLOSE);
	if(ret<0){
		printf("ioctl close error \n");
		return -1;
	}
	return ret;
}

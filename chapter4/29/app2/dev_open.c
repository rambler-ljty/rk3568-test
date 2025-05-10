/*
 * 设备打开函数实现
 * 用于打开定时器设备文件
 */

#include<stdio.h>
#include "timerlib.h"

/* 打开设备文件函数 */
int dev_open()
{
	int fd;  // 文件描述符
	// 以读写方式打开设备文件
	fd=open("/dev/test",O_RDWR,0777);
	if(fd<0){
		printf("file open error \n");
	}
	return fd;
}

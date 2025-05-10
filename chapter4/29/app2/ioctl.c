/*
 * 定时器测试程序
 * 使用定时器库函数演示定时器的基本操作
 * 包括：打开设备、设置时间、启动定时器、修改时间、关闭定时器等
 */

#include<stdio.h>
#include "timerlib.h"

int main(int argc,char *argv[])
{
	int fd;  // 文件描述符

	// 打开设备文件
	fd=dev_open();
	
	// 设置定时器时间为1000毫秒
	timer_set(fd,1000);
	// 启动定时器
	timer_open(fd);
	// 等待3秒
	sleep(3);
	
	// 修改定时器时间为3000毫秒
	timer_set(fd,3000);
	// 等待7秒
	sleep(7);
	
	// 关闭定时器
	timer_close(fd);
	// 关闭设备文件
	close(fd);
}

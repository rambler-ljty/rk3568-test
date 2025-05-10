/*
 * 这是一个用户空间测试程序，用于测试字符设备驱动的定时器功能
 * 该程序通过ioctl命令控制内核定时器的开启、关闭和时间设置
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>

/* 定义ioctl命令，需要与内核模块中的定义保持一致 */
#define TIME_OPEN _IO('L',0)     // 打开定时器
#define TIME_CLOSE _IO('L',1)    // 关闭定时器
#define TIME_SET _IOW('L',2,int) // 设置定时器时间

int main(int argc,char *argv[])
{
	int fd;  // 文件描述符

	// 打开设备文件
	fd=open("/dev/test",O_RDWR,0777);

	if(fd<0)
	{
		printf("file open error\n");	
	}
	
	// 设置定时器时间为1000毫秒并启动定时器
	ioctl(fd,TIME_SET,1000);
	ioctl(fd,TIME_OPEN);
	sleep(3);  // 等待3秒

	// 修改定时器时间为3000毫秒
	ioctl(fd,TIME_SET,3000);
	sleep(7);  // 等待7秒
	ioctl(fd,TIME_CLOSE);

	// 关闭设备文件
	close(fd);
}

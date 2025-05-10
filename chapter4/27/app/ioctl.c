/*
 * 这是一个测试程序，用于测试字符设备驱动的ioctl功能
 * 该程序演示了如何使用ioctl命令与设备驱动进行通信
 * 支持两种操作模式：write和read
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<string.h>

/* 定义ioctl命令，需要与驱动中的定义保持一致 */
#define CMD_TEST0 _IO('L',0)     // 无参数的命令
#define CMD_TEST1 _IOW('L',1,int)// 写命令，带参数
#define CMD_TEST2 _IOR('L',2,int)// 读命令，带参数

int main(int argc,char *argv[])
{
	int fd;          // 文件描述符
	int val;         // 用于存储读取的值

	// 打开设备节点
	fd=open("/dev/test",O_RDWR);
	if(fd<0)
	{
		printf("file open error\n");	
	}
	
	// 根据命令行参数执行不同的ioctl操作
	if(!strcmp(argv[1],"write")){
		// 执行写命令，传递参数1
		ioctl(fd,CMD_TEST1,1);
	}
	else if(!strcmp(argv[1],"read")){
		// 执行读命令，获取设备返回的值
		ioctl(fd,CMD_TEST2,&val);
		printf("val is %d\n",val);
	}

	// 关闭设备
	close(fd);
	return 0;
}

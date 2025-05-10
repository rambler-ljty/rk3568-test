/*
 * 这是一个用户空间测试程序，用于测试字符设备驱动的ioctl功能
 * 该程序通过ioctl命令向内核空间发送结构体参数
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<string.h>

/* 定义ioctl命令，需要与内核模块中的定义保持一致 */
#define CMD_TEST0 _IOW('L',0,int)

/* 定义参数结构体，需要与内核模块中的定义保持一致 */
struct args{
	int a;    // 参数a
	int b;    // 参数b
	int c;    // 参数c
};

int main(int argc,char *argv[])
{
	int fd;           // 文件描述符
	struct args test; // 测试参数结构体

	// 初始化测试参数
	test.a=1;
	test.b=2;
	test.c=3;

	// 打开设备文件
	fd=open("/dev/test",O_RDWR,0777);

	if(fd<0)
	{
		printf("file open error\n");	
	}
	
	// 通过ioctl发送结构体参数到内核空间
	ioctl(fd,CMD_TEST0,&test);

	// 关闭设备文件
	close(fd);
}

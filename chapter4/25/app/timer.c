/*
 * 这是一个测试程序，用于测试计时器设备驱动
 * 该程序每秒读取一次设备中的计数值并打印
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

int main(int argc,char *argv[])
{
	int fd;          // 文件描述符
	int count;       // 计数值

	// 打开设备节点
	fd=open("/dev/test",O_RDWR);
	
	// 主循环：每秒读取一次计数值
	while(1)
	{
		// 从设备读取计数值
		read(fd,&count,sizeof(count));
		// 延时1秒
		sleep(1);
		// 打印计数值
		printf("num is %d \n",count);
	}

	return 0;
}

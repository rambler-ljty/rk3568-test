/*
 * 这是一个测试程序，用于测试字符设备驱动的poll机制
 * 该程序使用poll函数监控设备状态，实现超时检测
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<poll.h>

int main(int argc,char *argv[])
{
	int fd;                  // 文件描述符
	char buf1[32]={0};      // 读取缓冲区
	char buf2[32]={0};      // 备用缓冲区
	struct pollfd fds[1];   // poll结构体数组
	int ret;                // poll返回值

	// 打开设备节点
	fd=open("/dev/test",O_RDWR);
	if(fd<0){
		printf("file open error\n");
		return fd;
	}

	// 设置poll结构体
	fds[0].fd=fd;           // 要监控的文件描述符
	fds[0].events=POLLIN;   // 监控可读事件

	printf("read before \n");
	// 循环监控设备状态
	while(1)
	{
		// 调用poll函数，超时时间3秒
		ret=poll(fds,1,3000);
		if(!ret){
			printf("time out!!\n");  // 超时处理
		}else if(fds[0].revents==POLLIN)  // 设备可读
		{
			read(fd,buf1,sizeof(buf1));
			printf("buf1 is %s \n",buf1);
			sleep(1);           // 每秒读取一次
		}
	}
	printf("read after \n");

	// 关闭设备
	close(fd);

	return 0;
}

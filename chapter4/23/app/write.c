/*
 * 这是一个测试程序，用于测试字符设备驱动的写入功能
 * 该程序会打开设备节点，写入数据"nihao"，触发异步通知
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main(int argc,char *argv[])
{
	int fd;                  // 文件描述符
	char buf1[32]={0};      // 备用缓冲区
	char buf2[32]="nihao";  // 写入缓冲区，包含要写入的数据

	// 打开设备节点
	fd=open("/dev/test",O_RDWR);
	if(fd<0){
		perror("file open error\n");
		return fd;
	}

	// 写入数据到设备
	printf("write before \n");
	write(fd,buf2,sizeof(buf2));
	printf("write after \n");

	// 关闭设备
	close(fd);

	return 0;
}

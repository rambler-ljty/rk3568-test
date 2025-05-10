/*
 * 这是一个测试程序，用于测试字符设备驱动的读取功能
 * 该程序会打开设备节点，读取数据并打印
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc,char *argv[])
{
	int fd;                  // 文件描述符
	char buf1[32]={0};      // 读取缓冲区
	char buf2[32]={0};      // 备用缓冲区

	// 打开设备节点
	fd=open("/dev/test",O_RDWR);
	if(fd<0){
		printf("file open error\n");
		return fd;
	}

	// 读取设备数据
	printf("read before \n");
	read(fd,buf1,sizeof(buf1));
	printf("buf1 is %s    \n",buf1);
	printf("read after \n");

	// 关闭设备
	close(fd);

	return 0;
}

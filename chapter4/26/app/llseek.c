/*
 * 这是一个测试程序，用于测试字符设备驱动的文件定位功能
 * 该程序演示了如何使用lseek函数进行文件定位，以及读写操作
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

int main(int argc,char *argv[])
{
	int fd;                      // 文件描述符
	unsigned int off;            // 文件偏移量
	char readbuf[13]={0};        // 第一次读取的缓冲区
	char readbuf1[19]={0};       // 第二次读取的缓冲区

	// 打开设备节点
	fd=open("/dev/test",O_RDWR,666);
	if(fd<0)
	{
		printf("file open error\n");	
	}
	
	// 写入数据"hello world"
	write(fd,"hello world",13);
	// 获取当前文件偏移量
	off=lseek(fd,0,SEEK_CUR);
	printf("off is %d\n",off);

	// 将文件偏移量设置为文件开始
	off=lseek(fd,0,SEEK_SET);
	printf("off is %d\n",off);

	// 读取数据
	read(fd,readbuf,sizeof(readbuf));
	printf("read is %s\n",readbuf);

	// 获取当前文件偏移量
	off=lseek(fd,0,SEEK_CUR);
	printf("off is %d\n",off);

	// 尝试将文件偏移量设置为-1（应该失败）
	off=lseek(fd,-1,SEEK_SET);
	printf("off is %d\n",off);

	// 写入数据"Linux"
	write(fd,"Linux",6);
	// 获取当前文件偏移量
	off=lseek(fd,0,SEEK_CUR);
	printf("off is %d\n",off);

	// 将文件偏移量设置为文件开始
	off=lseek(fd,0,SEEK_SET);
	printf("off is %d\n",off);

	// 读取数据
	read(fd,readbuf1,sizeof(readbuf1));
	printf("read is %s\n",readbuf1);

	// 获取当前文件偏移量
	off=lseek(fd,0,SEEK_CUR);
	printf("off is %d\n",off);

	// 关闭设备
	close(fd);
	return 0;
}

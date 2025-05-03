#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

/**
 * @brief 主函数，用于向设备文件写入特定字符串
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 成功返回0，失败返回错误码
 */
int main(int argc,char *argv[])
{
	int fd;  // 文件描述符
	char str1[10]={0};  // 缓冲区
	
	// 打开设备文件，以读写方式
	fd=open(argv[1],O_RDWR);
	if(fd<0){
		perror("file open error\n");
		return fd;
	}

	// 根据第二个参数写入不同的字符串
	if(strcmp(argv[2],"topeet")==0){
		write(fd,"topeet",10);  // 写入"topeet"字符串
	}
	
	if(strcmp(argv[2],"itop")==0){
		write(fd,"itop",10);  // 写入"itop"字符串
	}

	// 关闭文件
	close(fd);

	return 0;
}

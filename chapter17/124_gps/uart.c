/*
 * UART 串口配置模块
 * 功能：配置串口通信参数
 */

#include <stdio.h>      // 标准输入输出库
#include <termios.h>    // 终端控制结构体定义
#include <string.h>     // 字符串处理函数
#include <fcntl.h>      // 文件控制选项
#include <unistd.h>     // UNIX 标准函数定义

/**
 * 配置串口通信参数
 * 该函数用于设置串口的波特率、数据位、校验位和停止位等参数
 * 
 * @param fd 串口文件描述符
 * @param speed 波特率（9600 或 115200）
 * @param bits 数据位数（7 或 8）
 * @param check 校验位类型（'N'无校验，'O'奇校验，'E'偶校验）
 * @param stop 停止位数（1 或 2）
 * @return 0 成功，-1 获取属性失败，-2 设置属性失败
 */
int set_uart(int fd,int speed,int bits,char check,int stop){
	struct termios newtio,oldtio;  // 新的和旧的终端属性结构体

	// 获取当前终端属性
	if(tcgetattr(fd,&oldtio)!=0){
		printf("获取串口属性失败\n");
		return -1;
	}
	
	// 清零新属性结构体
	bzero(&newtio,sizeof(newtio));

	// 设置本地连接和接收使能
	newtio.c_cflag |= CLOCAL | CREAD;
	// 清除数据位大小设置
	newtio.c_cflag &= ~CSIZE;

	// 设置数据位数
	switch(bits){
		case 7:
			newtio.c_cflag |= CS7;  // 7位数据位
			break;
		case 8:
			newtio.c_cflag |= CS8;  // 8位数据位
			break;
	}

	// 设置校验位
	switch(check){
		case 'O':  // 奇校验
			newtio.c_cflag |= PARENB;   // 使能校验位
			newtio.c_cflag |= PARODD;   // 奇校验
			newtio.c_iflag |=(INPCK|ISTRIP);  // 使能输入校验和去除校验位
			break;
		case 'E':  // 偶校验
			newtio.c_cflag |= PARENB;   // 使能校验位
			newtio.c_cflag &= ~PARODD;  // 偶校验
			newtio.c_iflag |=(INPCK|ISTRIP);  // 使能输入校验和去除校验位
			break;
		case 'N':  // 无校验
			newtio.c_cflag &= ~PARENB;  // 禁用校验位
			break;
	}

	// 设置波特率
	switch(speed){
		case 9600:
			cfsetispeed(&newtio,B9600);    // 设置输入波特率为9600
			cfsetospeed(&newtio,B9600);    // 设置输出波特率为9600
			break;
		case 115200:
			cfsetispeed(&newtio,B115200);  // 设置输入波特率为115200
			cfsetospeed(&newtio,B115200);  // 设置输出波特率为115200
			break;
	}

	// 设置停止位
	switch(stop){
		case 1:
			newtio.c_cflag &= ~CSTOPB;  // 1个停止位
			break;
		case 2:
			newtio.c_cflag |= CSTOPB;   // 2个停止位
			break;
	}
	
	// 清空输入缓冲区
	tcflush(fd,TCIFLUSH);

	// 应用新的串口配置
	if(tcsetattr(fd,TCSANOW,&newtio)!=0){
		printf("设置串口属性失败\n");
		return -2;
	}

	return 0;  // 配置成功
}



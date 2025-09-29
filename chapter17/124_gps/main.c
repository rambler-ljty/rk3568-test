/*
 * GPS 应用程序主函数
 * 功能：通过串口接收 GPS 数据并解析显示
 */

#include <stdio.h>      // 标准输入输出库
#include <termios.h>    // 终端控制结构体定义
#include <string.h>     // 字符串处理函数
#include <fcntl.h>      // 文件控制选项
#include <unistd.h>     // UNIX 标准函数定义
#include <sys/types.h>  // 系统数据类型定义
#include <sys/stat.h>   // 文件状态结构体定义
#include "gps.h"        // GPS 模块头文件

/**
 * 主函数 - GPS 数据接收和解析程序
 * 程序流程：
 * 1. 打开串口设备
 * 2. 配置串口参数
 * 3. 等待 GPS 模块初始化
 * 4. 读取 GPS 数据
 * 5. 解析并显示 GPS 信息
 * 6. 关闭串口
 * 
 * @return 0 成功，-1 打开串口失败
 */
int main(){
	int fd;                    // 串口文件描述符
	char buf[128];             // GPS 数据接收缓冲区
	int count;                 // 接收到的字节数
	struct gprmc_data data;    // GPS 数据结构体

	// 打开串口设备文件 /dev/ttyS9
	// O_RDWR: 读写模式
	// O_NOCTTY: 不将此设备作为控制终端
	// O_NDELAY: 非阻塞模式
	fd=open("/dev/ttyS9",O_RDWR | O_NOCTTY| O_NDELAY);
	
	if(fd<0){
		printf("打开串口失败: /dev/ttyS9\n");
		return -1;
	}

	// 配置串口参数：115200波特率，8数据位，无校验，1停止位
	// 这是大多数 GPS 模块的标准通信参数
	set_uart(fd,115200,8,'N',1);

	// 等待 GPS 模块初始化和卫星信号锁定
	// GPS 模块通常需要几秒钟时间来获取卫星信号
	printf("等待 GPS 模块初始化...\n");
	sleep(2);

	// 从串口读取 GPS 数据
	// GPS 模块会持续发送 NMEA 格式的数据包
	count=read(fd,buf,sizeof(buf));
	
	// 清零 GPS 数据结构体
	memset(&data,0,sizeof(data));

	// 解析 GPS 数据并显示结果
	printf("开始解析 GPS 数据...\n");
	get_gps_data(buf,&data);

	// 等待数据发送完成
	sleep(2);

	// 关闭串口设备
	close(fd);
	printf("GPS 数据接收完成\n");

	return 0;  // 程序正常结束
}	


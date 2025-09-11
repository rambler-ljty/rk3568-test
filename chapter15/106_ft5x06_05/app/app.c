/*
 * FT5X06触摸屏测试应用程序
 * 
 * 本程序用于测试FT5X06触摸屏驱动的功能
 * 通过读取输入设备文件来获取触摸坐标信息
 * 
 * 功能特性:
 * - 读取输入设备事件
 * - 解析触摸坐标数据
 * - 实时显示X、Y坐标
 * 
 */

#include <fcntl.h>        // 文件控制操作
#include <linux/input.h>  // 输入子系统头文件
#include <unistd.h>       // Unix标准定义
#include <stdio.h>        // 标准输入输出

/**
 * main - 主函数
 * @argc: 命令行参数个数
 * @argv: 命令行参数数组
 * 
 * 打开输入设备文件，循环读取触摸事件并显示坐标
 * 
 * 返回值: 成功返回0，失败返回-1
 */
int main(int argc,char *argv[]){
	int fd;                    // 文件描述符
	struct input_event event;  // 输入事件结构体

	// 打开输入设备文件（event4对应触摸屏设备）
	fd=open("/dev/input/event4",O_RDONLY);
	if(fd<0){
		perror("open");
		return -1;
	}

	// 循环读取输入事件
	while(1){
		// 读取输入事件
		if(read(fd,&event,sizeof(event))!=sizeof(event)){
			perror("read");
			close(fd);
			return -1;
		}
	
		// 处理绝对坐标事件（触摸坐标）
		if(event.type==EV_ABS){
			if(event.code==ABS_X){
				// 显示X坐标（注意：原代码有错误，应该是%d）
				printf("x=%d\n",event.value);
			}
			else if(event.code==ABS_Y){
				// 显示Y坐标
				printf("y=%d\n",event.value);
			}
		}
	}

	return 0;
}

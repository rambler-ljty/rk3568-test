/*
 * app.c - Linux输入设备事件监听应用程序
 * 
 * 这个应用程序用于监听Linux输入设备产生的事件，特别是按键事件。
 * 程序会持续读取输入设备文件，解析事件数据并打印相关信息。
 */

#include<stdio.h>           // 标准输入输出库
#include<fcntl.h>          // 文件控制库
#include<unistd.h>         // UNIX标准库
#include<linux/input.h>    // Linux输入事件结构体定义

/*
 * main - 主函数
 * 
 * 功能：
 * 1. 打开输入设备文件
 * 2. 持续监听输入事件
 * 3. 解析按键事件并打印状态
 * 
 * 返回值：
 * 0 - 正常退出
 * -1 - 打开设备失败
 * -2 - 读取事件失败
 */
int main(){
	int fd;                           // 文件描述符
	int ret;                          // 返回值
	struct input_event event;         // 输入事件结构体

	// 打开输入设备文件，以读写模式打开
	fd=open("/dev/input/event4",O_RDWR);
	if(fd<0){
		printf("open error\n");
		return -1;
	}

	// 无限循环，持续监听输入事件
	while(1){
		// 读取输入事件数据
		ret=read(fd,&event,sizeof(struct input_event));
		if(ret<0){
			printf("read error\n");
			return -2;
		}

		// 检查事件类型是否为按键事件
		if(event.type==EV_KEY){
			// 检查按键代码是否为数字键1
			if(event.code==KEY_1){
				// 检查按键状态
				if(event.value==1){
					printf("value is 1\n");    // 按键按下
				}else if(event.value==0){
					printf("value is 0\n");    // 按键释放
				}
			}
		}
	}

	return 0;
}

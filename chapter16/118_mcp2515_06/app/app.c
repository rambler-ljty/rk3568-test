/*
 * MCP2515 CAN 控制器用户空间测试程序
 * 
 * 本程序用于测试 MCP2515 CAN 控制器的发送和接收功能
 * 通过字符设备接口与内核驱动进行通信
 * 
 * 功能：
 * - 打开 MCP2515 设备文件
 * - 发送 CAN 消息
 * - 接收 CAN 消息
 * - 打印接收到的数据
 * 
 * CAN 消息格式（13 字节）：
 * [0]: 0x66 - 标准帧标识符高字节
 * [1]: 0x08 - 标准帧标识符低字节
 * [2]: 0x22 - 数据长度（8 字节）
 * [3]: 0x33 - 数据字节 0
 * [4]: 0x08 - 数据字节 1
 * [5-12]: 0x01-0x08 - 数据字节 2-9
 */

#include <stdio.h>      // 标准输入输出头文件
#include <sys/types.h>  // 系统数据类型定义
#include <sys/stat.h>   // 文件状态头文件
#include <fcntl.h>      // 文件控制头文件
#include <unistd.h>     // UNIX 标准定义

/**
 * main - 主函数
 * @argc: 命令行参数个数
 * @argv: 命令行参数数组
 * 
 * 测试 MCP2515 CAN 控制器的发送和接收功能
 * 
 * 返回值: 成功返回 0，失败返回 -1
 */
int main(int argc, char *argv[])
{
	int fd;     // 文件描述符
	int i;      // 循环变量

	// CAN 消息发送缓冲区（13 字节）
	// 格式：标准帧 ID + 数据长度 + 8 字节数据
	char w_buf[13] = {0x66, 0x08, 0x22, 0x33, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

	// CAN 消息接收缓冲区（13 字节）
	char r_buf[13] = {0};

	// 打开 MCP2515 设备文件
	fd = open("/dev/mcp2515", O_RDWR);
	if(fd < 0){
		printf("open /dev/mcp2515 error\n");
		return -1;
	}

	// 发送 CAN 消息
	printf("Sending CAN message...\n");
	write(fd, w_buf, sizeof(w_buf));

	// 接收 CAN 消息
	printf("Receiving CAN message...\n");
	read(fd, r_buf, sizeof(r_buf));

	// 打印接收到的数据
	printf("Received data:\n");
	for(i = 0; i < 13; i++){
		printf("r_buf[%d] is %d (0x%02X)\n", i, r_buf[i], r_buf[i]);
	}

	// 关闭设备文件
	close(fd);

	return 0;
}

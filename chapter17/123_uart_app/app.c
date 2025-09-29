/*
 * UART 串口通信应用程序
 * 功能：通过串口发送和接收数据
 */

#include <stdio.h>      // 标准输入输出库
#include <termios.h>    // 终端控制结构体定义
#include <string.h>     // 字符串处理函数
#include <fcntl.h>      // 文件控制选项
#include <unistd.h>     // UNIX 标准函数定义

/**
 * 配置串口参数
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
		printf("tcgetattr oldtio error\n");
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
		printf("tcsetattr newtio error\n");
		return -2;
	}

	return 0;  // 配置成功

}


/**
 * 主函数 - UART 串口通信测试程序
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组，argv[1] 为要发送的字符串
 * @return 0 成功，-1 打开串口失败
 */
int main(int argc,char *argv[]){
	int fd;           // 串口文件描述符
	char buf[128];    // 接收缓冲区
	int count;        // 接收到的字节数

	// 检查命令行参数
	if(argc < 2){
		printf("用法: %s <要发送的字符串>\n", argv[0]);
		return -1;
	}

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
	set_uart(fd,115200,8,'N',1);

	// 发送命令行参数中的字符串
	write(fd,argv[1],strlen(argv[1]));
	printf("已发送: %s\n", argv[1]);
	
	// 等待1秒让数据发送完成
	sleep(1);

	// 读取串口返回的数据
	count=read(fd,buf,sizeof(buf));
	buf[count]='\0';  // 添加字符串结束符

	printf("接收到: %s\n",buf);

	// 关闭串口
	close(fd);

	return 0;  // 程序正常结束

}	


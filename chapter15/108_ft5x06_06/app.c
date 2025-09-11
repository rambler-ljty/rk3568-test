/*
 * FT5X06触摸屏I2C用户空间测试应用程序
 * 
 * 本程序通过用户空间I2C接口直接与FT5X06触摸屏控制器通信
 * 实现了寄存器读写功能，用于测试I2C通信和硬件功能
 * 
 * 功能特性:
 * - 用户空间I2C通信
 * - 寄存器读写操作
 * - 硬件功能测试
 * - 错误处理和调试
 * 
 */

#include <stdio.h>        // 标准输入输出
#include <linux/i2c.h>    // I2C内核接口
#include <linux/i2c-dev.h> // I2C设备接口
#include <sys/ioctl.h>    // 系统调用接口
#include <fcntl.h>        // 文件控制操作
#include <string.h>       // 字符串操作

/**
 * ft5x06_read_reg - 读取FT5X06寄存器
 * @fd: I2C设备文件描述符
 * @slave_addr: I2C从设备地址
 * @reg_addr: 要读取的寄存器地址
 * 
 * 通过用户空间I2C接口读取指定寄存器的值
 * 使用两个I2C消息：先发送寄存器地址，再读取数据
 * 
 * 返回值: 成功返回寄存器值，失败返回负错误码
 */
int ft5x06_read_reg(int fd,unsigned char slave_addr,unsigned  char reg_addr){
	unsigned char data;  // 存储读取的数据
	struct i2c_rdwr_ioctl_data i2c_msgs;  // I2C读写控制结构体
	int ret;

	// 构造I2C消息数组
	struct i2c_msg dev_msgs[]={
		[0]={
			.addr=slave_addr,           // I2C从设备地址
			.flags=0,                   // 写标志
			.len=sizeof(reg_addr),      // 寄存器地址长度
			.buf=&reg_addr,             // 寄存器地址缓冲区
		},
		[1]={
			.addr=slave_addr,           // I2C从设备地址
			.flags=I2C_M_RD,            // 读标志
			.len=sizeof(data),          // 数据长度
			.buf=&data,                 // 数据缓冲区
		}
	};

	// 设置I2C消息控制结构体
	i2c_msgs.msgs=dev_msgs;  // 指向消息数组
	i2c_msgs.nmsgs=2;        // 消息数量
	
	// 执行I2C读写操作
	ret=ioctl(fd,I2C_RDWR,&i2c_msgs);

	if(ret<0){
		printf("read error\n");
		return ret;
	}

	return data;
}

/**
 * ft5x06_write_reg - 写入FT5X06寄存器
 * @fd: I2C设备文件描述符
 * @slave_addr: I2C从设备地址
 * @reg_addr: 要写入的寄存器地址
 * @data: 要写入的数据缓冲区
 * @len: 数据长度
 * 
 * 通过用户空间I2C接口向指定寄存器写入数据
 * 将寄存器地址和数据组合成一个I2C消息发送
 */
void ft5x06_write_reg(int fd,unsigned char slave_addr,unsigned char reg_addr,unsigned char *data,int len){
	unsigned char buff[256];  // 数据缓冲区
	struct i2c_rdwr_ioctl_data i2c_msgs;  // I2C读写控制结构体
	int ret;

	// 构造I2C消息
	struct i2c_msg dev_msgs[]={
		[0]={
			.addr=slave_addr,           // I2C从设备地址
			.flags=0,                   // 写标志
			.len=len+1,                 // 总长度（地址+数据）
			.buf=buff,                  // 数据缓冲区
		}
	};

	// 组合寄存器地址和数据
	buff[0]=reg_addr;                  // 第一个字节是寄存器地址
	memcpy(&buff[1],data,len);         // 复制数据到缓冲区

	// 设置I2C消息控制结构体
	i2c_msgs.msgs=dev_msgs;  // 指向消息数组
	i2c_msgs.nmsgs=1;        // 消息数量

	// 执行I2C读写操作
	ret=ioctl(fd,I2C_RDWR,&i2c_msgs);
	if(ret<0){
		printf("write error\n");
	}
}

/**
 * main - 主函数
 * @argc: 命令行参数个数
 * @argv: 命令行参数数组
 * 
 * 打开I2C设备，测试寄存器读写功能
 * 
 * 处理流程:
 * 1. 打开I2C设备文件
 * 2. 向寄存器0x80写入数据0x55
 * 3. 读取寄存器0x80的值并显示
 * 
 * 返回值: 成功返回0，失败返回负错误码
 */
int main(int argc, char *argv[])
{
	int fd;                    // I2C设备文件描述符
	int ID_G_THGROUP;          // 存储读取的寄存器值

	// 打开I2C设备文件（I2C总线1）
	fd=open("/dev/i2c-1",O_RDWR);
	if(fd<0){
		printf("open error\n");
		return fd;
	}

	// 测试寄存器写入功能
	unsigned char data=0x55;  // 要写入的数据
	ft5x06_write_reg(fd,0x38,0x80,&data,1);  // 向寄存器0x80写入0x55

	// 测试寄存器读取功能
	ID_G_THGROUP=ft5x06_read_reg(fd,0x38,0x80);  // 读取寄存器0x80的值
	printf("ID_G_THGROUP is 0x%02x\n",ID_G_THGROUP);  // 显示读取结果

	return 0;
}

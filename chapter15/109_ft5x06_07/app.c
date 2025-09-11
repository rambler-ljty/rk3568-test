/*
 * FT5X06触摸屏I2C用户空间测试应用程序 (简化版本)
 * 
 * 本程序通过简化的用户空间I2C接口与FT5X06触摸屏控制器通信
 * 使用I2C_SLAVE_FORCE设置从设备地址，然后直接进行读写操作
 * 
 * 功能特性:
 * - 简化的用户空间I2C通信
 * - 直接读写操作
 * - 寄存器读写测试
 * - 硬件功能验证
 * 
 */

#include <stdio.h>        // 标准输入输出
#include <linux/i2c.h>    // I2C内核接口
#include <linux/i2c-dev.h> // I2C设备接口
#include <sys/ioctl.h>    // 系统调用接口
#include <sys/types.h>    // 系统类型定义
#include <sys/stat.h>     // 文件状态定义
#include <fcntl.h>        // 文件控制操作
#include <string.h>       // 字符串操作
#include <unistd.h>       // Unix标准定义

/**
 * ft5x06_read_reg - 读取FT5X06寄存器
 * @fd: I2C设备文件描述符
 * @reg_addr: 要读取的寄存器地址
 * 
 * 通过简化的I2C接口读取指定寄存器的值
 * 先写入寄存器地址，再读取数据
 * 
 * 注意: 这种方法假设I2C从设备地址已经通过ioctl设置
 */
void ft5x06_read_reg(int fd,unsigned  char reg_addr){
	unsigned char rd_data[1];  // 数据缓冲区
	rd_data[0]=reg_addr;       // 设置要读取的寄存器地址

	// 先写入寄存器地址
	write(fd,rd_data,1);
	// 再读取寄存器数据
	read(fd,rd_data,1);

	// 显示读取的寄存器值
	printf("reg value is %x\n",rd_data[0]);
}

/**
 * ft5x06_write_reg - 写入FT5X06寄存器
 * @fd: I2C设备文件描述符
 * @reg_addr: 要写入的寄存器地址
 * @data: 要写入的数据
 * 
 * 通过简化的I2C接口向指定寄存器写入数据
 * 将寄存器地址和数据组合后一次性写入
 * 
 * 注意: 这种方法假设I2C从设备地址已经通过ioctl设置
 */
void ft5x06_write_reg(int fd,unsigned char reg_addr,unsigned char data){
	unsigned char wr_data[2];  // 写入数据缓冲区
	wr_data[0]=reg_addr;       // 第一个字节是寄存器地址
	wr_data[1]=data;           // 第二个字节是要写入的数据
	
	// 一次性写入寄存器地址和数据
	write(fd,wr_data,2);
}

/**
 * main - 主函数
 * @argc: 命令行参数个数
 * @argv: 命令行参数数组
 * 
 * 打开I2C设备，设置从设备地址，测试寄存器读写功能
 * 
 * 处理流程:
 * 1. 打开I2C设备文件
 * 2. 设置I2C从设备地址为0x38
 * 3. 向寄存器0x80写入数据0x66
 * 4. 读取寄存器0x80的值并显示
 * 
 * 返回值: 成功返回0，失败返回负错误码
 */
int main(int argc, char *argv[])
{
	int fd;  // I2C设备文件描述符

	// 打开I2C设备文件（I2C总线1）
	fd=open("/dev/i2c-1",O_RDWR);
	if(fd<0){
		printf("open error\n");
		return fd;
	}

	// 设置I2C从设备地址为0x38（FT5X06的I2C地址）
	ioctl(fd,I2C_SLAVE_FORCE,0x38);
	
	// 测试寄存器写入功能：向寄存器0x80写入0x66
	ft5x06_write_reg(fd,0x80,0x66);

	// 测试寄存器读取功能：读取寄存器0x80的值
	ft5x06_read_reg(fd,0x80);

	return 0;
}

/*
 * MCP2515 CAN 控制器用户空间 SPI 通信程序
 * 
 * 本程序通过 Linux SPI 设备接口直接与 MCP2515 CAN 控制器进行通信
 * 使用 spidev 驱动提供的 ioctl 接口进行 SPI 数据传输
 * 
 * 功能：
 * - 初始化 SPI 设备接口
 * - 配置 SPI 通信参数（模式、速度、位宽）
 * - 发送 SPI 命令到 MCP2515
 * - 读取 MCP2515 状态寄存器
 * - 复位 MCP2515 芯片
 * - 配置 CAN 控制寄存器
 * 
 * SPI 命令说明：
 * - 0xC0: 复位命令
 * - 0x03: 读取命令
 * - 0x02: 写入命令
 * 
 * 寄存器说明：
 * - 0x0E: CANSTAT - CAN 状态寄存器
 * - 0x0F: CANCTRL - CAN 控制寄存器
 */

#include <stdio.h>              // 标准输入输出头文件
#include <sys/ioctl.h>          // ioctl 系统调用头文件
#include <linux/spi/spidev.h>   // SPI 设备接口头文件
#include <sys/types.h>          // 系统数据类型定义
#include <sys/stat.h>           // 文件状态头文件
#include <fcntl.h>              // 文件控制头文件
#include <string.h>             // 字符串处理头文件

// MCP2515 SPI 命令定义
#define RESET 0xc0              // 复位命令
#define CANSTAT 0x0e            // CAN 状态寄存器地址
#define READ 0X03               // 读取命令
#define CANCTRL 0x0f            // CAN 控制寄存器地址
#define WRITE 0x02              // 写入命令

// 全局变量定义
int fd;                         // SPI 设备文件描述符
int mode = SPI_MODE_0;          // SPI 模式（模式 0：CPOL=0, CPHA=0）
int bits = 8;                   // 每字比特数
int speed = 10000000;           // SPI 传输速度（10 MHz）
int delay;                      // 传输延迟（微秒）

/**
 * spi_init - 初始化 SPI 设备接口
 * 
 * 打开 SPI 设备文件并配置 SPI 通信参数
 * 
 * 返回值: 成功返回 0，失败返回 -1
 */
int spi_init(void)
{
	int ret;
	
	// 打开 SPI 设备文件（spidev0.0 表示 SPI 总线 0，设备 0）
	fd = open("/dev/spidev0.0", O_RDWR);
	if(fd < 0){
		printf("open /dev/spidev0.0 error\n");
		return -1;
	}
	
	// 设置 SPI 模式
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if(ret == -1)
		printf("cannot set spi mode\n");
	
	// 读取 SPI 模式（验证设置）
	ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if(ret == -1)
		printf("cannot get spi mode\n");

	// 设置每字比特数
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if(ret == -1)
		printf("无法设置每字比特数\n");

	// 读取每字比特数（验证设置）
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if(ret == -1)
		printf("无法获取每字比特数\n");

	// 设置最大传输速度
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if(ret == -1)
		printf("无法设置最大传输速度\n");

	// 打印 SPI 配置信息
	printf("SPI 模式：0x%x\n", mode);
	printf("每字比特数：%d\n", bits);
	printf("最大速度：%d Hz(%d KHz)\n", speed, speed/1000);

	return 0;
}

/**
 * transfer - SPI 数据传输函数
 * @fd: SPI 设备文件描述符
 * @tx: 发送数据缓冲区
 * @rx: 接收数据缓冲区
 * @len: 传输数据长度
 * 
 * 通过 ioctl 系统调用进行 SPI 数据传输
 * 
 * 返回值: 成功返回 0，失败返回 -1
 */
int transfer(int fd, char *tx, char *rx, int len)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,        // 发送缓冲区地址
		.rx_buf = (unsigned long)rx,        // 接收缓冲区地址
		.len = len,                         // 传输数据长度
		.delay_usecs = delay,               // 传输延迟（微秒）
		.speed_hz = speed,                  // 传输速度（Hz）
		.bits_per_word = bits,              // 每字比特数
	};

	// 执行 SPI 传输（发送 1 个消息）
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret < 1){
		printf("无法发送SPI 消息\n");
		return -1;
	}

	return 0;
}

/**
 * main - 主函数
 * @argc: 命令行参数个数
 * @argv: 命令行参数数组
 * 
 * 测试 MCP2515 CAN 控制器的 SPI 通信功能
 * 
 * 测试流程：
 * 1. 初始化 SPI 设备接口
 * 2. 发送复位命令到 MCP2515
 * 3. 读取 CAN 状态寄存器
 * 4. 写入 CAN 控制寄存器
 * 5. 再次读取 CAN 状态寄存器
 * 
 * 返回值: 成功返回 0，失败返回 -1
 */
int main(int argc, char *argv[])
{
	// 定义 SPI 命令和缓冲区
	char reset_cmd[1] = {RESET};                    // 复位命令
	char rd_canstat[2] = {READ, CANSTAT};           // 读取 CAN 状态寄存器命令
	char canstat[4] = {0};                          // CAN 状态寄存器数据缓冲区
	char wr_canctrl[] = {WRITE, CANCTRL, 0x00};     // 写入 CAN 控制寄存器命令

	// 初始化 SPI 设备接口
	spi_init();

	// 发送复位命令到 MCP2515
	printf("Sending reset command to MCP2515...\n");
	transfer(fd, reset_cmd, NULL, sizeof(reset_cmd));

	// 读取 CAN 状态寄存器
	printf("Reading CAN status register...\n");
	transfer(fd, rd_canstat, canstat, sizeof(canstat));
	printf("CAN 状态为 %x\n", canstat[2]);

	// 清空状态寄存器缓冲区
	memset(canstat, 0, sizeof(canstat));

	// 写入 CAN 控制寄存器（设置为正常模式）
	printf("Writing CAN control register...\n");
	transfer(fd, wr_canctrl, NULL, sizeof(wr_canctrl));
	
	// 注意：这里应该重新读取状态寄存器来验证写入结果
	// 但代码中直接打印了 canstat[3]，此时 canstat 已经被 memset 清零
	printf("CAN 状态为 %x\n", canstat[3]);

	return 0;
}

/*
 * MCP2515 CAN 控制器 SPI 驱动模块（CAN 通信版本）
 * 
 * 本模块实现了 MCP2515 CAN 控制器的 Linux 内核 SPI 驱动，并提供了字符设备接口
 * 用户空间程序可以通过标准的文件操作接口（open、read、write、close）与 MCP2515 进行通信
 * 
 * 功能特性：
 * - SPI 设备驱动框架
 * - 字符设备接口
 * - 设备树支持
 * - 用户空间访问接口
 * - SPI 通信功能（复位、寄存器读取、写入、位操作）
 * - CAN 控制器配置和初始化
 * - CAN 消息发送和接收功能
 * 
 * SPI 命令说明：
 * - 0xC0: 复位命令
 * - 0x03: 读取命令
 * - 0x02: 写入命令
 * - 0x05: 位修改命令
 */

#include <linux/init.h>     // 模块初始化和退出宏定义
#include <linux/module.h>   // 内核模块基本头文件
#include <linux/spi/spi.h>  // SPI 设备驱动相关头文件
#include <linux/cdev.h>     // 字符设备相关头文件
#include <linux/fs.h>       // 文件系统相关头文件
#include <linux/kdev_t.h>   // 设备号相关头文件
#include <linux/uaccess.h>  // 用户空间数据访问相关头文件

// MCP2515 寄存器地址定义
#define CNF1 0X2a        // 配置寄存器 1（位时序配置）
#define CNF2 0X29        // 配置寄存器 2（位时序配置）
#define CNF3 0X28        // 配置寄存器 3（位时序配置）
#define RXB0CTRL 0x60    // 接收缓冲区 0 控制寄存器
#define CANINTE 0X2b     // CAN 中断使能寄存器
#define CANCTRL 0xf      // CAN 控制寄存器

// CAN 通信相关寄存器
#define TXB0CTRL 0x30    // 发送缓冲区 0 控制寄存器
#define CANINTF 0x2c     // CAN 中断标志寄存器

// 全局变量定义
dev_t dev_num;                    // 设备号（主设备号+次设备号）
struct cdev mcp2515_cdev;         // 字符设备结构体
struct class *mcp2515_class;      // 设备类指针
struct device *mcp2515_device;    // 设备对象指针
struct spi_device *spi_dev;       // SPI 设备指针

/**
 * mcp2515_reset - MCP2515 复位函数
 * 
 * 向 MCP2515 发送复位命令（0xC0），使芯片进入复位状态
 * 复位后芯片会重新初始化所有内部寄存器
 * 
 * 返回值: 无
 */
void mcp2515_reset(void)
{
	int ret;
	char write_buf[] = {0xc0};  // MCP2515 复位命令
	
	// 通过 SPI 发送复位命令
	ret = spi_write(spi_dev, write_buf, sizeof(write_buf));
	if(ret < 0){
		printk("spi_write is error\n");
	}
}

/**
 * mcp2515_read_reg - 读取 MCP2515 寄存器
 * @reg: 要读取的寄存器地址
 * 
 * 通过 SPI 接口读取 MCP2515 指定寄存器的值
 * 使用 spi_write_then_read 函数先发送读取命令，再读取数据
 * 
 * 返回值: 成功返回寄存器值，失败返回负错误码
 */
char mcp2515_read_reg(char reg)
{
	char write_buf[] = {0x03, reg};  // 读取命令 + 寄存器地址
	char read_buf;
	int ret;

	// 先发送读取命令，再读取数据
	ret = spi_write_then_read(spi_dev, write_buf, sizeof(write_buf), &read_buf, sizeof(read_buf));
	if(ret < 0){
		printk("spi_write_then_read error\n");
		return ret;
	}

	return read_buf;
}

/**
 * mcp2515_write_reg - 写入 MCP2515 寄存器
 * @reg: 要写入的寄存器地址
 * @value: 要写入的值
 * 
 * 通过 SPI 接口向 MCP2515 指定寄存器写入值
 * 使用 spi_write 函数发送写入命令和数据
 * 
 * 返回值: 无
 */
void mcp2515_write_reg(char reg, char value)
{
	int ret;
	char write_buf[] = {0x02, reg, value};  // 写入命令 + 寄存器地址 + 数据

	// 通过 SPI 发送写入命令和数据
	ret = spi_write(spi_dev, write_buf, sizeof(write_buf));
	if(ret < 0){
		printk("mcp2515_write_reg error\n");
	}
}

/**
 * mcp2515_change_regbit - 修改 MCP2515 寄存器特定位
 * @reg: 要修改的寄存器地址
 * @mask: 位掩码，指定要修改的位
 * @value: 要设置的值
 * 
 * 通过 SPI 接口修改 MCP2515 指定寄存器的特定位
 * 使用 spi_write 函数发送位修改命令
 * 
 * 返回值: 无
 */
void mcp2515_change_regbit(char reg, char mask, char value)
{
	int ret;
	char write_buf[] = {0x05, reg, mask, value};  // 位修改命令 + 寄存器地址 + 掩码 + 值

	// 通过 SPI 发送位修改命令
	ret = spi_write(spi_dev, write_buf, sizeof(write_buf));
	if(ret < 0){
		printk("mcp2515_change_regbit error");
	}
}


/**
 * mcp2515_open - 字符设备打开函数
 * @inode: 文件节点结构体指针
 * @file: 文件结构体指针
 * 
 * 当用户空间程序调用 open() 系统调用时，内核会调用此函数
 * 在此函数中进行设备打开时的初始化工作
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
int mcp2515_open(struct inode *inode, struct file *file)
{
	return 0;
}

/**
 * mcp2515_read - 字符设备读取函数（CAN 消息接收）
 * @file: 文件结构体指针
 * @buf: 用户空间缓冲区指针
 * @size: 要读取的字节数
 * @offset: 文件偏移量指针
 * 
 * 当用户空间程序调用 read() 系统调用时，内核会调用此函数
 * 在此函数中实现从 MCP2515 接收 CAN 消息的逻辑
 * 
 * 接收流程：
 * 1. 等待接收中断标志（RX0IF）
 * 2. 从接收缓冲区读取 13 字节数据
 * 3. 清除接收中断标志
 * 4. 将数据复制到用户空间
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
ssize_t mcp2515_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	char r_kbuf[13] = {0};  // 接收缓冲区，13 字节 CAN 消息
	int i;
	int ret;

	// 等待接收中断标志（RX0IF = bit 0）
	while(!(mcp2515_read_reg(CANINTF) & (1<<0)));
	
	// 从接收缓冲区 0 读取 13 字节数据（0x61-0x6D）
	for(i = 0; i < sizeof(r_kbuf); i++){
		r_kbuf[i] = mcp2515_read_reg(0x61 + i);
	}

	// 清除接收中断标志（RX0IF）
	mcp2515_change_regbit(CANINTF, 0x01, 0x00);

	// 将数据从内核空间复制到用户空间
	ret = copy_to_user(buf, r_kbuf, size);
	if(ret){
		printk("copy_to_user r_kbuf is error\n");
		return -1;
	}
	
	return 0;
}

/**
 * mcp2515_write - 字符设备写入函数（CAN 消息发送）
 * @file: 文件结构体指针
 * @buf: 用户空间缓冲区指针
 * @size: 要写入的字节数
 * @offset: 文件偏移量指针
 * 
 * 当用户空间程序调用 write() 系统调用时，内核会调用此函数
 * 在此函数中实现向 MCP2515 发送 CAN 消息的逻辑
 * 
 * 发送流程：
 * 1. 清除发送中断标志
 * 2. 从用户空间复制数据到内核空间
 * 3. 将数据写入发送缓冲区 0（0x31-0x3D）
 * 4. 启动发送（TXREQ = 1）
 * 5. 等待发送完成中断（TX0IF）
 * 6. 清除发送中断标志
 * 
 * 返回值: 成功返回实际写入的字节数，失败返回负错误码
 */
ssize_t mcp2515_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	char w_kbuf[13] = {0};  // 发送缓冲区，13 字节 CAN 消息
	int i;
	int ret;
	
	// 清除发送中断标志（TX0IF, TX1IF）
	mcp2515_change_regbit(CANINTF, 0x03, 0x03);
	
	// 将数据从用户空间复制到内核空间
	ret = copy_from_user(w_kbuf, buf, size);
	if(ret){
		printk("copy_from_user w_kbuf is error\n");
		return -1;
	}

	// 将数据写入发送缓冲区 0（0x31-0x3D）
	for(i = 0; i < sizeof(w_kbuf); i++){
		mcp2515_write_reg(0x31 + i, w_kbuf[i]);
	}

	// 启动发送（设置 TXREQ 位）
	mcp2515_change_regbit(TXB0CTRL, 0x08, 0x08);
	
	// 等待发送完成中断（TX0IF = bit 2）
	while(!(mcp2515_read_reg(CANINTF) & (1<<2)));

	// 清除发送中断标志（TX0IF）
	mcp2515_change_regbit(CANINTF, 0x04, 0x00);

	return size;
}

/**
 * mcp2515_release - 字符设备释放函数
 * @inode: 文件节点结构体指针
 * @file: 文件结构体指针
 * 
 * 当用户空间程序调用 close() 系统调用时，内核会调用此函数
 * 在此函数中进行设备关闭时的清理工作
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
int mcp2515_release(struct inode *inode, struct file *file)
{
	return 0;
}


/**
 * mcp2515_fops - 字符设备文件操作结构体
 * 
 * 定义了字符设备支持的文件操作函数指针
 * 用户空间程序通过系统调用访问设备时，内核会调用相应的函数
 */
struct file_operations mcp2515_fops = {
	.open = mcp2515_open,      // 设备打开函数
	.read = mcp2515_read,      // 设备读取函数（CAN 消息接收）
	.write = mcp2515_write,    // 设备写入函数（CAN 消息发送）
	.release = mcp2515_release, // 设备释放函数
};


/**
 * mcp2515_probe - MCP2515 设备探测函数
 * @spi: SPI 设备结构体指针
 * 
 * 当设备树中匹配到 MCP2515 设备时，内核会调用此函数
 * 在此函数中进行设备的初始化工作，包括：
 * - 分配字符设备号
 * - 初始化字符设备结构体
 * - 创建设备类
 * - 创建设备文件节点
 * - 复位 MCP2515 芯片
 * - 读取并打印芯片 ID
 * - 配置 CAN 控制器参数
 * - 设置接收缓冲区和中断
 * - 启动 CAN 控制器
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
int mcp2515_probe(struct spi_device *spi)
{
	int ret, value;
	printk("This is mcp2515_probe\n");
	spi_dev = spi;  // 保存 SPI 设备指针

	// 分配字符设备号（主设备号由内核自动分配，次设备号从 0 开始，分配 1 个设备）
	ret = alloc_chrdev_region(&dev_num, 0, 1, "mcp2515");
	if(ret < 0){
		printk("alloc_chrdev_region error\n");
		return ret;
	}

	// 初始化字符设备结构体
	cdev_init(&mcp2515_cdev, &mcp2515_fops);
	mcp2515_cdev.owner = THIS_MODULE;

	// 将字符设备添加到内核
	ret = cdev_add(&mcp2515_cdev, dev_num, 1);
	if(ret < 0){
		printk("cdev_add error\n");
		unregister_chrdev_region(dev_num, 1);  // 添加失败时释放设备号
		return ret;
	}

	// 创建设备类（在 /sys/class/ 下创建目录）
	mcp2515_class = class_create(THIS_MODULE, "spi_to_can");
	if(IS_ERR(mcp2515_class)){
		printk("mcp2515_class error\n");
		cdev_del(&mcp2515_cdev);
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(mcp2515_class);
	}

	// 创建设备文件（在 /dev/ 下创建设备节点）
	mcp2515_device = device_create(mcp2515_class, NULL, dev_num, NULL, "mcp2515");
	if(IS_ERR(mcp2515_device)){
		printk("mcp2515_device error\n");
		class_destroy(mcp2515_class);
		cdev_del(&mcp2515_cdev);
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(mcp2515_device);
	}
	
	// 复位 MCP2515 芯片
	mcp2515_reset();
	
	// 读取芯片 ID 寄存器（0x0E）并打印
	value = mcp2515_read_reg(0x0e);
	printk("value is %x\n", value);

	// 配置 CAN 控制器参数
	// CNF1: 设置位时序参数（SJW=1, BRP=1）
	mcp2515_write_reg(CNF1, 0X01);
	// CNF2: 设置位时序参数（BTLMODE=1, SAM=1, PS1=3, PS2=1）
	mcp2515_write_reg(CNF2, 0Xb1);
	// CNF3: 设置位时序参数（PS2=5）
	mcp2515_write_reg(CNF3, 0X05);
	
	// 配置接收缓冲区 0（RXB0CTRL: 接收所有消息，启用过滤器）
	mcp2515_write_reg(RXB0CTRL, 0X60);
	// 使能 CAN 中断（CANINTE: 使能接收中断和错误中断）
	mcp2515_write_reg(CANINTE, 0X05);
	// 启动 CAN 控制器（CANCTRL: 设置正常模式）
	mcp2515_change_regbit(CANCTRL, 0Xe0, 0X40);

	// 再次读取芯片 ID 寄存器验证配置
	value = mcp2515_read_reg(0x0e);
	printk("value is %x\n", value);

	return 0;
}

/**
 * mcp2515_remove - MCP2515 设备移除函数
 * @spi: SPI 设备结构体指针
 * 
 * 当设备被移除或模块卸载时，内核会调用此函数
 * 在此函数中进行资源的清理工作，包括：
 * - 销毁设备文件节点
 * - 销毁设备类
 * - 删除字符设备
 * - 释放设备号
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
static int mcp2515_remove(struct spi_device *spi)
{
	// 销毁设备文件
	device_destroy(mcp2515_class, dev_num);
	// 销毁设备类
	class_destroy(mcp2515_class);
	// 从内核中删除字符设备
	cdev_del(&mcp2515_cdev);
	// 释放设备号
	unregister_chrdev_region(dev_num, 1);
	return 0;
}


/**
 * mcp2515_of_match_table - 设备树匹配表
 * 
 * 定义了与设备树中 compatible 属性匹配的字符串
 * 当设备树中的设备节点包含 "my-mcp2515" 时，会匹配到此驱动
 */
static const struct of_device_id mcp2515_of_match_table[] = {
	{.compatible = "my-mcp2515"},  // 设备树兼容性字符串
	{}  // 数组结束标记
};

/**
 * mcp2515_id_table - SPI 设备 ID 表
 * 
 * 定义了通过 SPI 总线匹配的设备 ID
 * 当 SPI 设备名称匹配时会加载此驱动
 */
static const struct spi_device_id mcp2515_id_table[] = {
	{"mcp2515", 0},  // 设备名称和驱动数据
	{}  // 数组结束标记
};

/**
 * spi_mcp2515 - SPI 驱动结构体
 * 
 * 定义了 MCP2515 的 SPI 驱动，包含：
 * - probe: 设备探测函数
 * - remove: 设备移除函数
 * - driver: 驱动信息（名称、所有者、设备树匹配表）
 * - id_table: SPI 设备 ID 匹配表
 */
static struct spi_driver spi_mcp2515 = {
	.probe = mcp2515_probe,        // 设备探测函数指针
	.remove = mcp2515_remove,      // 设备移除函数指针
	.driver = {
		.name = "mcp2515",                    // 驱动名称
		.owner = THIS_MODULE,                 // 模块所有者
		.of_match_table = mcp2515_of_match_table,  // 设备树匹配表
	},
	.id_table = mcp2515_id_table,    // SPI 设备 ID 表
};


/**
 * mcp2515_init - 模块初始化函数
 * 
 * 当模块被加载时调用，注册 SPI 驱动到内核
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
static int __init mcp2515_init(void)
{
	int ret;

	// 注册 SPI 驱动到内核
	ret = spi_register_driver(&spi_mcp2515);
	if(ret < 0){
		printk("spi_register_driver error\n");
		return ret;
	}
	
	return ret;
}


/**
 * mcp2515_exit - 模块退出函数
 * 
 * 当模块被卸载时调用，从内核中注销 SPI 驱动
 */
static void __exit mcp2515_exit(void)
{
	// 从内核中注销 SPI 驱动
	spi_unregister_driver(&spi_mcp2515);
}


// 指定模块的初始化和退出函数
module_init(mcp2515_init);
module_exit(mcp2515_exit);

// 指定模块许可证为 GPL
MODULE_LICENSE("GPL");


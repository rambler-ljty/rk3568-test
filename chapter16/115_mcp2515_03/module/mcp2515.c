/*
 * MCP2515 CAN 控制器 SPI 驱动模块（字符设备版本）
 * 
 * 本模块实现了 MCP2515 CAN 控制器的 Linux 内核 SPI 驱动，并提供了字符设备接口
 * 用户空间程序可以通过标准的文件操作接口（open、read、write、close）与 MCP2515 通信
 * 
 * 功能特性：
 * - SPI 设备驱动框架
 * - 字符设备接口
 * - 设备树支持
 * - 用户空间访问接口
 */

#include <linux/init.h>     // 模块初始化和退出宏定义
#include <linux/module.h>   // 内核模块基本头文件
#include <linux/spi/spi.h>  // SPI 设备驱动相关头文件
#include <linux/cdev.h>     // 字符设备相关头文件
#include <linux/fs.h>       // 文件系统相关头文件
#include <linux/kdev_t.h>   // 设备号相关头文件
#include <linux/device.h>   // 设备类相关头文件

// 全局变量定义
dev_t dev_num;                    // 设备号（主设备号+次设备号）
struct cdev mcp2515_cdev;         // 字符设备结构体
struct class *mcp2515_class;      // 设备类指针
struct device *mcp2515_device;    // 设备对象指针
static struct spi_device *spi_dev; // SPI 设备指针

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
 * mcp2515_read - 字符设备读取函数
 * @file: 文件结构体指针
 * @buf: 用户空间缓冲区指针
 * @size: 要读取的字节数
 * @offset: 文件偏移量指针
 * 
 * 当用户空间程序调用 read() 系统调用时，内核会调用此函数
 * 在此函数中实现从 MCP2515 读取数据的逻辑
 * 
 * 返回值: 成功返回实际读取的字节数，失败返回负错误码
 */
ssize_t mcp2515_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

/**
 * mcp2515_write - 字符设备写入函数
 * @file: 文件结构体指针
 * @buf: 用户空间缓冲区指针
 * @size: 要写入的字节数
 * @offset: 文件偏移量指针
 * 
 * 当用户空间程序调用 write() 系统调用时，内核会调用此函数
 * 在此函数中实现向 MCP2515 写入数据的逻辑
 * 
 * 返回值: 成功返回实际写入的字节数，失败返回负错误码
 */
ssize_t mcp2515_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	return 0;
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
	.read = mcp2515_read,      // 设备读取函数
	.write = mcp2515_write,    // 设备写入函数
	.release = mcp2515_release, // 设备释放函数
};

/**
 * mcp2515_probe - MCP2515 设备探测函数
 * @spi: SPI 设备结构体指针
 * 
 * 当设备树中匹配到 MCP2515 设备时，内核会调用此函数
 * 在此函数中进行设备的初始化工作，包括：
 * - 分配字符设备号
 * - 初始化字符设备
 * - 创建设备类
 * - 创建设备文件
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
int mcp2515_probe(struct spi_device *spi)
{
	int ret;
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
	
	return 0;
}

/**
 * mcp2515_remove - MCP2515 设备移除函数
 * @spi: SPI 设备结构体指针
 * 
 * 当设备被移除或模块卸载时，内核会调用此函数
 * 在此函数中进行资源的清理工作，包括：
 * - 销毁设备文件
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


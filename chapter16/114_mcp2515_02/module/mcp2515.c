/*
 * MCP2515 CAN 控制器 SPI 驱动模块
 * 
 * 本模块实现了 MCP2515 CAN 控制器的基本 SPI 驱动框架
 * MCP2515 是一个独立的 CAN 控制器，通过 SPI 接口与主控制器通信
 * 
 */

#include <linux/init.h>     // 模块初始化和退出宏定义
#include <linux/module.h>   // 内核模块基本头文件
#include <linux/spi/spi.h>  // SPI 设备驱动相关头文件

/**
 * mcp2515_probe - MCP2515 设备探测函数
 * @spi: SPI 设备结构体指针
 * 
 * 当设备树中匹配到 MCP2515 设备时，内核会调用此函数
 * 在此函数中进行设备的初始化工作
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
static int mcp2515_probe(struct spi_device *spi)
{
	printk("This is mcp2515 probe\n");
	return 0;
}

/**
 * mcp2515_remove - MCP2515 设备移除函数
 * @spi: SPI 设备结构体指针
 * 
 * 当设备被移除或模块卸载时，内核会调用此函数
 * 在此函数中进行资源的清理工作
 * 
 * 返回值: 成功返回 0，失败返回负错误码
 */
static int mcp2515_remove(struct spi_device *spi)
{
	return 0;
}

/**
 * mcp2515_of_match_table - 设备树匹配表
 * 
 * 定义了与设备树中 compatible 属性匹配的字符串
 * 当设备树中的设备节点包含 "my-mcp2515" 时，会匹配到此驱动
 */
static const struct of_device_id mcp2515_of_match_table[]={
	{.compatible="my-mcp2515"},	// 设备树兼容性字符串
	{}  // 数组结束标记
};

/**
 * mcp2515_id_table - SPI 设备 ID 表
 * 
 * 定义了通过 SPI 总线匹配的设备 ID
 * 当 SPI 设备名称匹配时会加载此驱动
 */
static const struct spi_device_id mcp2515_id_table[]={
	{"mcp2515",0},  // 设备名称和驱动数据
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
static struct spi_driver spi_mcp2515={
	.probe = mcp2515_probe,        // 设备探测函数指针
	.remove = mcp2515_remove,      // 设备移除函数指针
	.driver={
		.name="mcp2515",                    // 驱动名称
		.owner=THIS_MODULE,                 // 模块所有者
		.of_match_table=mcp2515_of_match_table,  // 设备树匹配表
	},
	.id_table=mcp2515_id_table,    // SPI 设备 ID 表
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
	ret=spi_register_driver(&spi_mcp2515);
	if(ret<0){
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


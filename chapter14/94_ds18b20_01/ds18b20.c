/*
 * DS18B20温度传感器Linux内核驱动模块
 * 功能：实现DS18B20温度传感器的平台驱动框架
 */

#include <linux/init.h>        // 模块初始化和退出宏定义
#include <linux/module.h>      // 内核模块相关宏定义
#include <linux/platform_device.h>  // 平台设备驱动框架
#include <linux/of.h>          // 设备树支持

/**
 * ds18b20_probe - 平台驱动探测函数
 * @dev: 平台设备结构体指针
 * 
 * 当内核发现匹配的设备时，会调用此函数进行设备初始化
 * 返回值：0表示成功，负值表示失败
 */
int ds18b20_probe(struct platform_device *dev){
	printk("This is probe \n");  // 打印探测信息
	return 0;                    // 返回成功
}

/**
 * ds18b20_match_table - 设备树匹配表
 * 用于在设备树中匹配compatible属性为"ds18b20"的节点
 * 当内核扫描设备树时，会查找匹配的compatible字符串
 */
const struct of_device_id ds18b20_match_table[]={
	{.compatible="ds18b20"},  // 匹配设备树中的ds18b20节点
	{},                       // 空项，表示匹配表结束
	
};

/**
 * ds18b20_driver - 平台驱动结构体
 * 定义了驱动的基本信息，包括名称、匹配表、探测函数等
 */
struct platform_driver ds18b20_driver={
	.driver={
		.owner=THIS_MODULE,                    // 模块所有者
		.name="ds18b20",                       // 驱动名称
		.of_match_table=ds18b20_match_table,   // 设备树匹配表
	},
	.probe=ds18b20_probe,                     // 探测函数指针
};

/**
 * ds18b20_init - 模块初始化函数
 * 在模块加载时被调用，负责注册平台驱动到内核
 * 返回值：0表示成功，负值表示失败
 */
static int __init ds18b20_init(void){
	int ret;

	// 注册平台驱动到内核
	ret=platform_driver_register(&ds18b20_driver);
	if(ret<0){
		printk("platform_driver_register error\n");  // 注册失败时打印错误信息
		return -1;
	}

	return 0;  // 注册成功
}

/**
 * ds18b20_exit - 模块退出函数
 * 在模块卸载时被调用，负责注销平台驱动
 */
static void __exit ds18b20_exit(void){
	// 从内核中注销平台驱动
	platform_driver_unregister(&ds18b20_driver);
}

// 模块初始化和退出宏，指定加载和卸载时调用的函数
module_init(ds18b20_init);    // 模块加载时调用ds18b20_init
module_exit(ds18b20_exit);    // 模块卸载时调用ds18b20_exit

// 模块许可证声明
MODULE_LICENSE("GPL");


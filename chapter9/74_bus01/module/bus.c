/*
 * bus.c - Linux内核模块示例：演示自定义总线的创建和注册
 * 
 * 本模块展示了如何创建和注册一个自定义的总线类型，包括实现
 * 总线匹配函数和探测函数。总线是Linux设备驱动模型中的核心概念，
 * 用于连接设备和驱动程序。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口
#include<linux/device.h>    // 提供设备驱动模型相关接口

/**
 * mybus_match - 总线匹配函数
 * 
 * 功能：判断设备和驱动程序是否匹配
 * 当有新的设备或驱动注册到总线时，内核会调用此函数来判断是否匹配
 * 
 * 参数：
 * @dev: 指向设备结构体的指针
 * @drv: 指向设备驱动结构体的指针
 * 
 * 返回值：
 * 1表示匹配，0表示不匹配
 * 
 * 注意：这里使用简单的字符串比较，实际应用中可能需要更复杂的匹配逻辑
 */
int mybus_match(struct device *dev,struct device_driver *drv)
{
	// 比较设备名称和驱动名称是否相同
	// dev_name(dev)获取设备的名称
	// drv->name是驱动的名称
	return(strcmp(dev_name(dev),drv->name)==0);
}

/**
 * mybus_probe - 总线探测函数
 * 
 * 功能：当设备和驱动匹配成功后，调用此函数进行设备的探测和初始化
 * 
 * 参数：
 * @dev: 指向设备结构体的指针
 * 
 * 返回值：0表示成功
 * 
 * 注意：此函数会调用驱动程序的probe函数来完成具体的设备初始化
 */
int mybus_probe(struct device *dev)
{
	// 获取设备对应的驱动程序
	struct device_driver *drv = dev->driver;
	
	// 如果驱动程序有probe函数，则调用它
	if(drv->probe)
		drv->probe(dev);
	return 0;
}

/**
 * mybus - 自定义总线类型结构体
 * 
 * 功能：定义一个新的总线类型，包含总线的名称和操作函数
 * 
 * 成员：
 * .name: 总线名称，会在/sys/bus/下创建对应的目录
 * .match: 指向匹配函数的指针
 * .probe: 指向探测函数的指针
 */
struct bus_type mybus={
	.name="mybus",        // 总线名称，会创建/sys/bus/mybus/目录
	.match=mybus_match,   // 匹配函数指针
	.probe=mybus_probe,   // 探测函数指针
};

/**
 * bus_init - 模块初始化函数
 * 
 * 功能：
 * 1. 注册自定义总线类型到内核
 * 2. 在sysfs中创建总线相关的目录结构
 * 
 * 返回值：0表示成功
 */
static int bus_init(void)
{
	int ret;
	
	// 注册总线类型到内核
	// bus_register会：
	// 1. 将总线注册到内核的总线系统中
	// 2. 在/sys/bus/下创建mybus目录
	// 3. 创建drivers和devices子目录
	ret=bus_register(&mybus);
	return 0;
}

/**
 * bus_exit - 模块退出函数
 * 
 * 功能：
 * 注销自定义总线类型，清理相关资源
 * 
 * 注意：bus_unregister会自动清理所有注册到该总线的设备和驱动
 */
static void bus_exit(void)
{
	// 注销总线类型
	// bus_unregister会：
	// 1. 从内核总线系统中移除总线
	// 2. 清理/sys/bus/mybus/目录
	// 3. 清理所有相关的设备和驱动
	bus_unregister(&mybus);
}

// 模块初始化函数注册
module_init(bus_init);    // 注册初始化函数，模块加载时调用
module_exit(bus_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

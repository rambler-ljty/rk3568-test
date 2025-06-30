/*
 * driver.c - Linux内核模块示例：演示驱动程序的创建和注册
 * 
 * 本模块展示了如何创建和注册一个驱动程序到自定义总线上。驱动程序
 * 是Linux设备驱动模型中的核心组件，负责管理特定类型的设备，包括
 * 设备的探测、移除和操作等功能。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口
#include<linux/device.h>    // 提供设备驱动模型相关接口
#include<linux/sysfs.h>     // 提供sysfs相关接口

// 声明外部变量，引用在bus模块中定义的mybus总线
extern struct bus_type mybus;

/**
 * mydriver_remove - 驱动移除函数
 * 
 * 功能：当设备与驱动分离时，内核会调用此函数
 * 此函数负责清理驱动相关的资源，释放设备占用的内存等
 * 
 * 参数：
 * @dev: 指向设备结构体的指针
 * 
 * 返回值：0表示成功
 * 
 * 注意：此函数在设备卸载或驱动卸载时被调用
 */
int mydriver_remove(struct device *dev){
	// 打印调试信息，表示驱动正在移除设备
	printk("This is mydriver_remove\n");
	return 0;
}

/**
 * mydriver_probe - 驱动探测函数
 * 
 * 功能：当总线发现匹配的设备时，内核会调用此函数
 * 此函数负责初始化设备，分配资源，设置设备状态等
 * 
 * 参数：
 * @dev: 指向设备结构体的指针
 * 
 * 返回值：0表示成功，负值表示失败
 * 
 * 注意：此函数在设备与驱动绑定成功时被调用
 */
int mydriver_probe(struct device *dev){
	// 打印调试信息，表示驱动正在探测设备
	printk("This is mydriver_probe\n");
	return 0;
}

/**
 * mydriver - 自定义驱动结构体
 * 
 * 功能：定义一个驱动程序，包含驱动的基本信息和操作函数
 * 
 * 成员：
 * .name: 驱动名称，用于与设备进行匹配
 * .bus: 指向驱动所属的总线
 * .probe: 指向驱动探测函数的指针
 * .remove: 指向驱动移除函数的指针
 */
struct device_driver mydriver={
	.name="mydevice",              // 驱动名称，与设备名称匹配
	.bus=&mybus,                   // 指向mybus总线
	.probe=mydriver_probe,         // 驱动探测函数指针
	.remove=mydriver_remove,       // 驱动移除函数指针
};

/**
 * mydriver_init - 模块初始化函数
 * 
 * 功能：
 * 1. 注册自定义驱动程序到mybus总线
 * 2. 在sysfs中创建驱动相关的目录结构
 * 3. 触发总线的匹配机制，寻找匹配的设备
 * 
 * 返回值：0表示成功，负值表示失败
 */
static int mydriver_init(void)
{
	int ret;
	
	// 注册驱动程序到总线
	// driver_register会：
	// 1. 将驱动注册到指定的总线（mybus）
	// 2. 在/sys/bus/mybus/drivers/下创建mydevice目录
	// 3. 触发总线的匹配函数，寻找匹配的设备
	// 4. 如果找到匹配的设备，会调用驱动的probe函数
	ret=driver_register(&mydriver);
	return ret;
}

/**
 * mydriver_exit - 模块退出函数
 * 
 * 功能：
 * 注销自定义驱动程序，清理相关资源
 * 
 * 注意：driver_unregister会自动调用驱动的remove函数
 */
static void mydriver_exit(void)
{
	// 注销驱动程序
	// driver_unregister会：
	// 1. 从总线中移除驱动
	// 2. 清理/sys/bus/mybus/drivers/mydevice目录
	// 3. 对已绑定的设备调用remove函数
	// 4. 减少驱动的引用计数
	driver_unregister(&mydriver);
}

// 模块初始化函数注册
module_init(mydriver_init);    // 注册初始化函数，模块加载时调用
module_exit(mydriver_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

/*
 * device.c - Linux内核模块示例：演示设备的创建和注册
 * 
 * 本模块展示了如何创建和注册一个设备到自定义总线上。设备是Linux
 * 设备驱动模型中的核心概念，代表一个硬件设备或虚拟设备，需要
 * 与对应的驱动程序进行匹配和绑定。
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
 * myrelease - 设备释放函数
 * 
 * 功能：当设备的引用计数为0时，内核会自动调用此函数
 * 此函数负责清理设备相关的资源
 * 
 * 参数：
 * @dev: 指向设备结构体的指针
 * 
 * 注意：此函数是设备生命周期管理的最后一步
 */
void myrelease(struct device *dev)
{
	// 打印调试信息，表示设备正在被释放
	printk("This is myrelease\n");
}

/**
 * mydevice - 自定义设备结构体
 * 
 * 功能：定义一个设备，包含设备的基本信息和操作函数
 * 
 * 成员：
 * .init_name: 设备的初始名称，会在sysfs中显示
 * .bus: 指向设备所属的总线
 * .release: 指向设备释放函数的指针
 * .devt: 设备号，用于设备文件的创建
 */
struct device mydevice={
	.init_name="mydevice",           // 设备名称，会显示在/sys/bus/mybus/devices/mydevice
	.bus=&mybus,                     // 指向mybus总线
	.release=myrelease,              // 设备释放函数指针
	.devt=((255<<20|0)),             // 设备号：主设备号255，次设备号0
};

/**
 * device_init - 模块初始化函数
 * 
 * 功能：
 * 1. 注册自定义设备到mybus总线
 * 2. 在sysfs中创建设备相关的目录结构
 * 3. 触发总线的匹配机制，寻找对应的驱动程序
 * 
 * 返回值：0表示成功
 */
static int device_init(void)
{
	int ret;
	
	// 注册设备到总线
	// device_register会：
	// 1. 将设备注册到指定的总线（mybus）
	// 2. 在/sys/bus/mybus/devices/下创建mydevice目录
	// 3. 触发总线的匹配函数，寻找匹配的驱动程序
	// 4. 如果找到匹配的驱动，会调用总线的probe函数
	ret=device_register(&mydevice);
	return 0;
}

/**
 * device_exit - 模块退出函数
 * 
 * 功能：
 * 注销自定义设备，清理相关资源
 * 
 * 注意：device_unregister会自动调用设备的release函数
 */
static void device_exit(void)
{
	// 注销设备
	// device_unregister会：
	// 1. 从总线中移除设备
	// 2. 清理/sys/bus/mybus/devices/mydevice目录
	// 3. 减少设备的引用计数
	// 4. 当引用计数为0时，调用myrelease函数
	device_unregister(&mydevice);
}

// 模块初始化函数注册
module_init(device_init);    // 注册初始化函数，模块加载时调用
module_exit(device_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

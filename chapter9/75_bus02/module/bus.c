/*
 * bus.c - Linux内核模块示例：演示总线属性的创建和管理
 * 
 * 本模块在74_bus01的基础上，增加了总线属性的功能。展示了如何为总线
 * 创建自定义属性，通过sysfs提供用户空间访问总线信息的接口。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口
#include<linux/device.h>    // 提供设备驱动模型相关接口
#include<linux/sysfs.h>     // 提供sysfs相关接口

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
 */
int mybus_match(struct device *dev,struct device_driver *drv)
{
	// 比较设备名称和驱动名称是否相同
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
 * mybus_show - 总线属性读取函数
 * 
 * 功能：处理用户空间对总线属性的读取操作（如cat命令）
 * 
 * 参数：
 * @bus: 指向总线类型结构体的指针
 * @buf: 输出缓冲区，用于存储要返回给用户空间的数据
 * 
 * 返回值：写入缓冲区的字节数
 */
ssize_t mybus_show(struct bus_type *bus,char *buf)
{
	// 返回固定的字符串信息
	return sprintf(buf,"%s\n","mybus_show");
}

/**
 * mybus_attr - 总线属性结构体
 * 
 * 功能：定义总线的自定义属性，提供用户空间访问总线信息的接口
 * 
 * 成员：
 * .attr: 基本的属性信息（名称、权限等）
 * .show: 指向读取函数的指针
 * 
 * 注意：这里只实现了读取功能，没有实现写入功能
 */
struct bus_attribute mybus_attr={
	.attr = {
		.name ="value",   // 属性名称，会创建/sys/bus/mybus/value文件
		.mode =0664,      // 文件权限（所有者读写，组读写，其他用户只读）
	},
	.show =mybus_show,    // 读取函数指针
};

/**
 * bus_init - 模块初始化函数
 * 
 * 功能：
 * 1. 注册自定义总线类型到内核
 * 2. 为总线创建自定义属性文件
 * 3. 在sysfs中创建完整的目录结构
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
	
	// 为总线创建自定义属性文件
	// bus_create_file会：
	// 1. 在/sys/bus/mybus/下创建value文件
	// 2. 将mybus_attr与总线关联
	// 3. 提供用户空间访问接口
	ret=bus_create_file(&mybus,&mybus_attr);
	return 0;
}

/**
 * bus_exit - 模块退出函数
 * 
 * 功能：
 * 1. 删除总线的自定义属性文件
 * 2. 注销自定义总线类型
 * 3. 清理相关资源
 * 
 * 注意：清理顺序很重要，先删除属性文件，再注销总线
 */
static void bus_exit(void)
{
	// 删除总线的自定义属性文件
	// bus_remove_file会：
	// 1. 从/sys/bus/mybus/下删除value文件
	// 2. 清理属性与总线的关联
	bus_remove_file(&mybus,&mybus_attr);
	
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

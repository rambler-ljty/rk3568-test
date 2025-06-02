/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备相关接口
#include<linux/ioport.h>   // IO 资源管理相关接口

#define MEM_START_ADDR 0XFDD60000   // 内存资源起始地址
#define MEM_END_ADDR 0XFDD60004     // 内存资源结束地址
#define IRQ_NUMBER 101              // 中断号

// 定义平台设备需要的资源（内存和中断）
static struct resource my_resources[]={
    {
	.start=MEM_START_ADDR,   // 资源起始地址
	.end=MEM_END_ADDR,       // 资源结束地址
	.flags=IORESOURCE_MEM,   // 资源类型：内存
    },
    {
	.start=IRQ_NUMBER,       // 资源起始（中断号）
	.end=IRQ_NUMBER,         // 资源结束（中断号）
	.flags=IORESOURCE_IRQ,   // 资源类型：中断
    },

};

// 设备释放时的回调函数（必须实现，否则卸载设备时会有警告）
static void my_platform_device_release(struct device *dev)
{
    // 这里可以添加设备释放时的清理操作，目前为空
}

// 定义平台设备结构体
static struct platform_device my_platform_device={
    .name="my_platform_device",                  // 设备名称
    .id=-1,                                     // 设备ID，-1表示只有一个该类型设备
    .num_resources=ARRAY_SIZE(my_resources),     // 资源数量
    .resource=my_resources,                      // 资源数组
    .dev.release=my_platform_device_release,     // 设备释放回调
};

// 模块初始化函数
static int __init my_platform_device_init(void)
{
	int ret;
	// 注册平台设备
	ret=platform_device_register(&my_platform_device);
	if(ret){
		printk(KERN_ERR"Failed to register platform device\n");
		return ret;
	}

	printk(KERN_INFO"Platform device registered\n");

	return 0;
}

// 模块退出函数
static void __exit my_platform_device_exit(void)
{
	// 注销平台设备
	platform_device_unregister(&my_platform_device);
	printk(KERN_INFO"Platform device unregistered\n");
}

/* 注册模块的初始化和清理函数 */
module_init(my_platform_device_init);    // 注册初始化函数
module_exit(my_platform_device_exit);    // 注册清理函数

MODULE_LICENSE("GPL");   // 模块许可证
MODULE_AUTHOR("topeet");    // 模块作者


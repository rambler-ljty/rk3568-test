/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备驱动相关接口

/* 平台设备探测函数
 * 当内核发现匹配的平台设备时，会调用此函数
 * @pdev: 指向平台设备的指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_probe(struct platform_device *pdev)
{
	printk(KERN_INFO"my_platform_probe: Probing platform device\n");

	return 0;
}

/* 平台设备移除函数
 * 当设备被移除或驱动被卸载时调用
 * @pdev: 指向平台设备的指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_remove(struct platform_device *pdev)
{
	printk(KERN_INFO"my_platform_remove: Removing platform device\n");

	return 0;
}

/* 定义平台驱动结构体
 * 包含驱动名称、探测函数、移除函数等信息
 */
static struct platform_driver my_platform_driver={
    .probe=my_platform_probe,      // 设备探测函数
    .remove=my_platform_remove,    // 设备移除函数
    .driver={
	.name="my_platform_device",    // 驱动名称，需要与平台设备名称匹配
	.owner=THIS_MODULE,           // 模块所有者
    },
};

/* 模块初始化函数
 * 在模块加载时调用，注册平台驱动
 * 返回值: 成功返回0，失败返回负值
 */
static int __init my_platform_driver_init(void)
{
	int ret;
	// 注册平台驱动到内核
	ret=platform_driver_register(&my_platform_driver);
	if(ret){
		printk(KERN_ERR"Failed to register platform driver\n");
		return ret;
	}

	printk(KERN_INFO"my_platform_driver:Platform driver initialized\n");

	return 0;
}

/* 模块退出函数
 * 在模块卸载时调用，注销平台驱动
 */
static void __exit my_platform_driver_exit(void)
{
	platform_driver_unregister(&my_platform_driver);
	printk(KERN_INFO"my_platform_driver: Platform driver exited\n");
}

/* 注册模块的初始化和清理函数 */
module_init(my_platform_driver_init);    // 注册初始化函数
module_exit(my_platform_driver_exit);    // 注册清理函数

MODULE_LICENSE("GPL");   // 模块许可证
MODULE_AUTHOR("topeet");    // 模块作者


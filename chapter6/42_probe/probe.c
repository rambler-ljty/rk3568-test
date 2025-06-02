/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备驱动相关接口
#include<linux/ioport.h>              // IO资源管理相关接口

/* 平台设备探测函数
 * 当内核发现匹配的平台设备时，会调用此函数
 * 此函数演示了两种获取设备资源的方法
 * @pdev: 指向平台设备的指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_driver_probe(struct platform_device *pdev)
{
	struct resource *res_mem,*res_irq;

	/* 方法1：直接从设备资源数组中获取资源
	 * 检查设备是否至少有两个资源
	 */
	if(pdev->num_resources>=2){
		struct resource *res_mem= &pdev->resource[0];    // 获取内存资源
		struct resource *res_irq= &pdev->resource[0];    // 获取中断资源
	
		printk("Method 1: Memory Resource: start=0x%llx,end=0x%llx\n",res_mem->start,res_mem->end);
		printk("Method 1: IRQ Resource: number=%lld\n",res_irq->start);
	}

	/* 方法2：使用platform_get_resource函数获取资源
	 * 这是更推荐的方法，更安全且更灵活
	 */
	res_mem=platform_get_resource(pdev,IORESOURCE_MEM,0);    // 获取内存资源
	if(!res_mem){
		dev_err(&pdev->dev,"Failed to get memory resource\n");
		return -ENODEV;
	}
	
	res_irq=platform_get_resource(pdev,IORESOURCE_IRQ,0);    // 获取中断资源
	if(!res_irq){
		dev_err(&pdev->dev,"Failed to get IRQ resource\n");
		return -ENODEV;
	}

	printk("Method 2: Memory Resource: start=0x%llx,end=0x%llx\n",res_mem->start,res_mem->end);
	printk("Method 2: IRQ Resource: number=%lld\n",res_irq->start);

	return 0;
}

/* 平台设备移除函数
 * 当设备被移除或驱动被卸载时调用
 * @pdev: 指向平台设备的指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_driver_remove(struct platform_device *pdev)
{
	//remove
	return 0;
}

/* 定义平台驱动结构体
 * 包含驱动名称、探测函数、移除函数等信息
 */
static struct platform_driver my_platform_driver={
    .driver={
	.name="my_platform_device",    // 驱动名称，需要与平台设备名称匹配
	.owner=THIS_MODULE,           // 模块所有者
    },
    .probe=my_platform_driver_probe,      // 设备探测函数
    .remove=my_platform_driver_remove,    // 设备移除函数
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
		printk("Failed to register platform driver\n");
		return ret;
	}

	printk("Platform driver registered\n");
	return 0;
}

/* 模块退出函数
 * 在模块卸载时调用，注销平台驱动
 */
static void __exit my_platform_driver_exit(void)
{
	platform_driver_unregister(&my_platform_driver);
	printk("Platform driver unregistered\n");
}

/* 注册模块的初始化和清理函数 */
module_init(my_platform_driver_init);    // 注册初始化函数
module_exit(my_platform_driver_exit);    // 注册清理函数

MODULE_LICENSE("GPL");   // 模块许可证
MODULE_AUTHOR("topeet");    // 模块作者


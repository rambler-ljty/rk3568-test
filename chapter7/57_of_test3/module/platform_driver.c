/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备驱动相关接口
#include<linux/mod_devicetable.h>      // 设备表相关定义
#include<linux/of.h>                   // 设备树相关接口
#include<linux/gpio.h>                 // GPIO相关接口
#include<linux/of_irq.h>               // 设备树中断相关接口

// 设备树节点相关变量
struct device_node *mydevice_node;      // 设备树节点指针
int num;                                // 用于存储属性元素数量
int irq;                                // 中断号
struct irq_data *my_irq_data;           // 中断数据结构指针
u32 trigger_type;                       // 中断触发类型

/* 平台设备探测函数
 * 当设备与驱动匹配时，内核会调用此函数
 * @pdev: 指向平台设备的指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_probe(struct platform_device *pdev)
{
	printk(KERN_INFO"my_platform_probe: Probing platform device\n");

	// 通过节点名称查找设备树节点
	mydevice_node=of_find_node_by_name(NULL,"myirq");
	printk("mydevice_node is %s\n",mydevice_node->name);
	
	// 从设备树解析并映射中断
	irq=irq_of_parse_and_map(mydevice_node,0);
	printk("irq is %d\n",irq);

	// 获取中断数据结构
	my_irq_data=irq_get_irq_data(irq);

	// 获取中断触发类型
	trigger_type=irqd_get_trigger_type(my_irq_data);
	printk("trigger type is 0x%x\n",trigger_type);

	// 将GPIO号转换为中断号
	irq=gpio_to_irq(101);
	printk("irq is %d\n",irq);

	// 从设备树获取中断号
	irq=of_irq_get(mydevice_node,0);
	printk("irq is %d\n",irq);

	// 从平台设备获取中断号
	irq=platform_get_irq(pdev,0);
	printk("irq is %d\n",irq);

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

// 定义设备树匹配表，用于匹配设备树节点
const struct of_device_id of_match_table_id[] ={
	{.compatible="my_devicetree_irq"},     // 兼容性字符串
};

/* 定义平台驱动结构体
 * 包含驱动名称、探测函数、移除函数等信息
 */
static struct platform_driver my_platform_driver={
	.probe=my_platform_probe,          // 设备探测函数
	.remove=my_platform_remove,        // 设备移除函数
	.driver={
		.name="my_platform_device",    // 驱动名称，需要与平台设备名称匹配
		.owner=THIS_MODULE,           // 模块所有者
		.of_match_table=of_match_table_id,  // 设备树匹配表
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

MODULE_LICENSE("GPL");                   // 模块许可证
MODULE_AUTHOR("topeet");                 // 模块作者


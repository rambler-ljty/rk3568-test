/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备驱动相关接口
#include<linux/mod_devicetable.h>      // 设备表相关定义
#include<linux/of.h>                   // 设备树相关接口

// 设备树节点相关变量
struct device_node *mydevice_node;      // 设备树节点指针
const struct of_device_id *mynode_math; // 设备树匹配表指针

// 定义设备树匹配表
struct of_device_id mynode_of_match[]={
	{.compatible="my devicetree"},     // 兼容性字符串，用于匹配设备树节点
	{},                                // 结束标记
};

/* 平台设备探测函数
 * 当设备与驱动匹配时，内核会调用此函数
 * @pdev: 指向平台设备的指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_probe(struct platform_device *pdev)
{
	printk(KERN_INFO"my_platform_probe: Probing platform device\n");

	// 通过节点名称查找设备树节点
	mydevice_node=of_find_node_by_name(NULL,"myLed");
	printk("mydevice_node is %s\n",mydevice_node->name);

	// 通过完整路径查找设备树节点
	mydevice_node=of_find_node_by_path("/topeet/myLed");
	printk("mydevice_node is %s\n",mydevice_node->name);

	// 获取父节点
	mydevice_node=of_get_parent(mydevice_node);
	printk("myled's parent node is %s\n",mydevice_node->name);

	// 获取下一个子节点
	mydevice_node=of_get_next_child(mydevice_node,NULL);
	printk("myled's sibling node is %s\n",mydevice_node->name);

	// 通过兼容性字符串查找节点
	mydevice_node=of_find_compatible_node(NULL,NULL,"my devicetree");
	printk("mydevice_node is %s\n",mydevice_node->name);

	// 通过匹配表查找节点
	mydevice_node=of_find_matching_node_and_match(NULL,mynode_of_match,&mynode_math);
	printk("mydevice_node is %s\n",mydevice_node->name);

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
	{.compatible="my devicetree"},     // 兼容性字符串
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


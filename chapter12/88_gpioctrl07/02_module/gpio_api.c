#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备驱动相关接口
#include<linux/mod_devicetable.h>  
#include<linux/gpio/consumer.h>
#include<linux/gpio.h>

// 设备子节点数量
unsigned int count;
// 子节点句柄
struct fwnode_handle *child_fw=NULL;
// GPIO描述符数组
struct gpio_desc *led[2];
// 循环变量
int i=0;
// GPIO编号
int num=0;

/**
 * 平台设备初始化函数
 * @param dev 平台设备指针
 * @return 0成功，负值表示失败
 */
static int my_platform_probe(struct platform_device *dev)
{
	printk("This is my_platform_probe\n");

	// 获取设备子节点数量
	count=device_get_child_node_count(&dev->dev);

	printk("count is %d\n",count);

	// 遍历子节点，获取GPIO描述符并打印GPIO编号
	for(i=0;i<count;i++){
		child_fw=device_get_next_child_node(&dev->dev,child_fw);

		if(child_fw){
			led[i]=fwnode_get_named_gpiod(child_fw,"my-gpios",0,0,"LED");
		}

		num=desc_to_gpio(led[i]);
		printk("num is %d\n",num);
	}

	return 0;
}

/**
 * 平台设备移除函数
 * @param pdev 平台设备指针
 * @return 0成功
 */
static int my_platform_remove(struct platform_device *pdev)
{
	printk(KERN_INFO"my_platform_remove: Removing platform device\n");

	return 0;
}

// 设备匹配表
const struct of_device_id of_match_table_id[] ={
	 {.compatible="mygpio"},
};

// 平台驱动结构体
static struct platform_driver my_platform_driver={
    .probe=my_platform_probe,     
    .remove=my_platform_remove, 
    .driver={
	.name="my_platform_device",    // 驱动名称，需要与平台设备名称匹配
	.owner=THIS_MODULE,           // 模块所有者
	.of_match_table=of_match_table_id,
    },
};

// 模块初始化函数，注册平台驱动
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

// 模块退出函数，注销平台驱动
static void __exit my_platform_driver_exit(void)
{
	platform_driver_unregister(&my_platform_driver);
	printk(KERN_INFO"my_platform_driver: Platform driver exited\n");
}

module_init(my_platform_driver_init);    // 注册初始化函数
module_exit(my_platform_driver_exit);    // 注册清理函数

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("topeet");   

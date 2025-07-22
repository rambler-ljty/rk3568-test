#include <linux/module.h>    // 提供模块相关的功能
#include <linux/platform_device.h>      // 平台设备驱动相关接口
#include <linux/mod_devicetable.h>  
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>

// GPIO 描述符指针
struct gpio_desc *mygpio1;
// GPIO方向，电平，中断号
int dir,value,irq;

/**
 * 平台设备初始化函数
 * @param dev 平台设备指针
 * @return 0成功，负值表示失败
 */
int my_driver_probe(struct platform_device *dev)
{
	printk("This is mydriver_probe\n");
	mygpio1=gpiod_get_optional(&dev->dev,"my",0);

	if(mygpio1==NULL){
		printk("gpiod_get_optional error\n");
		return -1;
	}

	// 设置GPIO方向为输出，初始电平为0
	gpiod_direction_output(mygpio1,0);
	// 设置GPIO电平为高
	gpiod_set_value(mygpio1,1);

	// 获取GPIO方向
	dir=gpiod_get_direction(mygpio1);

	if(dir==GPIOF_DIR_IN){
		printk("dir is GPIOF_DIR_IN\n");
	}else if(dir==GPIOF_DIR_OUT){
		printk("dir is GPIOF_DIR_OUT\n");
	}

	// 获取GPIO电平值
	value=gpiod_get_value(mygpio1);
	printk("value is %d\n",value);

	// 获取GPIO对应的中断号
	irq=gpiod_to_irq(mygpio1);
	printk("irq is %d\n",irq);

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
    .probe=my_driver_probe,     
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
	gpiod_put(mygpio1);
	platform_driver_unregister(&my_platform_driver);
	printk(KERN_INFO"my_platform_driver: Platform driver exited\n");
}

module_init(my_platform_driver_init);    // 注册初始化函数
module_exit(my_platform_driver_exit);    // 注册清理函数

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("topeet");   

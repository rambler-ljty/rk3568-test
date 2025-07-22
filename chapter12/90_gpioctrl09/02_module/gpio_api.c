#include<linux/module.h>    // 提供模块相关的功能
#include<linux/platform_device.h>      // 平台设备驱动相关接口
#include<linux/mod_devicetable.h>  
#include<linux/gpio/consumer.h>
#include<linux/gpio.h>
#include<linux/device.h>

// pinctrl句柄
struct pinctrl *gpio_pinctrl;
// pinctrl状态1
struct pinctrl_state *func1_state;
// pinctrl状态2
struct pinctrl_state *func2_state;
// 返回值
int ret;

/**
 * 设备属性写函数，根据输入切换pinctrl状态
 * @param dev 设备指针
 * @param attr 设备属性指针
 * @param buf 输入缓冲区
 * @param count 输入长度
 * @return 写入字节数
 */
ssize_t selectmux_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
{
	unsigned long select;
	select = simple_strtoul(buf,NULL,10);
	if(select==1){
		pinctrl_select_state(gpio_pinctrl,func1_state);
	}else if(select==0)
	{
		pinctrl_select_state(gpio_pinctrl,func2_state);
	}
	return count;
}

DEVICE_ATTR_WO(selectmux);

/**
 * 获取pinctrl句柄并查找状态
 * @param dev 设备指针
 * @return 0成功，负值表示失败
 */
int pinctrl_get_and_lookstate(struct device *dev)
{
	gpio_pinctrl=pinctrl_get(dev);
	if(IS_ERR(gpio_pinctrl)){
		printk("pinctrl_get is error\n");
		return -1;
	}
	
	func1_state=pinctrl_lookup_state(gpio_pinctrl,"mygpio_func1");
	if(IS_ERR(func1_state)){
		printk("pinctrl_lookup_state is error\n");
		return -2;
	}

	func2_state=pinctrl_lookup_state(gpio_pinctrl,"mygpio_func2");
	if(IS_ERR(func2_state)){
		printk("pinctrl_lookup_state is error\n");
		return -3;
	}

	return 0;
}

/**
 * 平台设备初始化函数
 * @param dev 平台设备指针
 * @return 0成功，负值表示失败
 */
static int my_platform_probe(struct platform_device *dev)
{
	printk("This is my_platform_probe\n");

	pinctrl_get_and_lookstate(&dev->dev);

	device_create_file(&dev->dev,&dev_attr_selectmux);

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

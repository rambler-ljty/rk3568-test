/* 
 * 这是一个GPIO中断驱动程序示例
 * 功能：演示如何在Linux内核中注册和处理GPIO中断
 */

/* 包含必要的内核头文件 */
#include<linux/module.h>    // 模块相关的基本功能
#include<linux/init.h>      // 模块初始化和清理函数
#include<linux/gpio.h>      // GPIO相关功能
#include<linux/interrupt.h> // 中断处理相关功能

/* 定义GPIO引脚号 */
#define GPIO_PIN 101

/* 
 * GPIO中断处理函数
 * 当GPIO引脚检测到上升沿时，此函数被调用
 * @param irq: 中断号
 * @param dev_id: 设备ID指针，用于区分共享中断的设备
 * @return: 返回IRQ_HANDLED表示中断已被处理
 */
static irqreturn_t gpio_irq_handler(int irq,void *dev_id)
{
	// 打印中断发生信息
	printk(KERN_INFO "Interrupt occurred on GPIO %d\n",GPIO_PIN);
	printk(KERN_INFO "This is irq_handler\n");
	return IRQ_HANDLED;
}

/* 
 * 模块初始化函数
 * 在模块加载时被调用，完成以下工作：
 * 1. 将GPIO引脚映射到中断号
 * 2. 注册中断处理函数
 * 3. 设置中断触发方式为上升沿触发
 * @return: 成功返回0，失败返回负值
 */
static int __init interrupt_init(void)
{
	int irq_num;
	printk(KERN_INFO "Intializing GPIO Interrupt Driver\n");

	// 将GPIO引脚号转换为对应的中断号
	irq_num=gpio_to_irq(GPIO_PIN);
	printk(KERN_INFO "GPIO %d mapped to IRQ %d\n",GPIO_PIN,irq_num);
	
	// 注册中断处理函数
	// IRQF_TRIGGER_RISING表示上升沿触发
	if(request_irq(irq_num,gpio_irq_handler,IRQF_TRIGGER_RISING,"irq_test",NULL)!=0){
		printk(KERN_INFO "Failed to request IRQ %d\n",irq_num);
		gpio_free(GPIO_PIN);
		return -ENODEV;
	}

	return 0;
}

/* 
 * 模块退出函数
 * 在模块卸载时被调用，完成以下工作：
 * 1. 释放中断资源
 * 2. 清理相关资源
 */
static void __exit interrupt_exit(void)
{
	int irq_num=gpio_to_irq(GPIO_PIN);
	free_irq(irq_num,NULL);  // 释放中断
	printk(KERN_INFO "GPIO Interrupt Driver exited successfully\n");
}

// 注册模块的初始化和退出函数
module_init(interrupt_init);
module_exit(interrupt_exit);

// 模块许可证和作者信息
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

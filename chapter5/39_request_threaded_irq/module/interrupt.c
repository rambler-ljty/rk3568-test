/* 
 * 这是一个使用线程化中断(Threaded IRQ)的驱动程序示例
 * 功能：演示如何在Linux内核中使用线程化中断机制处理中断
 * 特点：将中断处理分为上半部和下半部，下半部在独立的内核线程中执行
 */

/* 包含必要的内核头文件 */
#include<linux/module.h>    // 模块相关的基本功能
#include<linux/init.h>      // 模块初始化和清理函数
#include<linux/gpio.h>      // GPIO相关功能
#include<linux/interrupt.h> // 中断处理相关功能
#include<linux/delay.h>     // 延时相关功能
#include<linux/workqueue.h> // 工作队列相关功能

/* 全局变量定义 */
int irq;                    // 中断号

/* 
 * 中断处理函数的下半部
 * 在独立的内核线程中执行，处理耗时操作
 * @param irq: 中断号
 * @param args: 设备ID指针
 * @return: 返回IRQ_HANDLED表示中断已被处理
 */
irqreturn_t test_work(int irq,void *args)
{
	msleep(1000);  // 延时1秒，模拟耗时操作
	printk("This is test_work\n");
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 
 * 中断处理函数的上半部
 * 在中断上下文中执行，处理紧急操作
 * @param irq: 中断号
 * @param args: 设备ID指针
 * @return: 返回IRQ_WAKE_THREAD表示需要唤醒下半部处理线程
 */
irqreturn_t test_interrupt(int irq,void *args)
{
	printk("This is test_interrupt\n");
	return IRQ_WAKE_THREAD;  // 唤醒下半部处理线程
}

/* 
 * 模块初始化函数
 * 在模块加载时被调用，完成以下工作：
 * 1. 将GPIO引脚映射到中断号
 * 2. 注册线程化中断处理函数
 * @return: 成功返回0，失败返回负值
 */
static int interrupt_irq_init(void)
{
	int ret;
	irq=gpio_to_irq(101);  // 将GPIO 101映射到中断号

	printk("irq is %d\n",irq);
	
	// 注册线程化中断处理函数，设置上半部和下半部处理函数
	ret=request_threaded_irq(irq,test_interrupt,test_work,IRQF_TRIGGER_RISING,"test",NULL);

	if(ret<0)
	{
		printk("request_irq is error\n");
		return -1;
	}

	return 0;
}

/* 
 * 模块退出函数
 * 在模块卸载时被调用，完成以下工作：
 * 1. 释放中断资源
 */
static void interrupt_irq_exit(void)
{
	free_irq(irq,NULL);  // 释放中断
	printk("bye bye\n");
}

// 注册模块的初始化和退出函数
module_init(interrupt_irq_init);
module_exit(interrupt_irq_exit);

// 模块许可证和作者信息
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

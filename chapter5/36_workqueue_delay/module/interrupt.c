/* 
 * 这是一个使用延迟工作队列(delayed workqueue)处理中断的驱动程序示例
 * 功能：演示如何在Linux内核中使用延迟工作队列机制处理中断
 * 特点：使用delayed_work实现延迟执行的工作队列，可以指定延迟时间
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
struct workqueue_struct *test_workqueue;  // 工作队列结构体指针
struct delayed_work test_workqueue_work;  // 延迟工作结构体

/* 
 * 工作队列处理函数
 * 当延迟工作被调度时，此函数被调用
 * @param work: 工作队列结构体指针
 */
void test_work(struct work_struct *work)
{
	msleep(1000);  // 延时1秒
	printk("This is test_work\n");
}

/* 
 * 中断处理函数
 * 当GPIO引脚检测到上升沿时，此函数被调用
 * @param irq: 中断号
 * @param args: 设备ID指针
 * @return: 返回IRQ_HANDLED表示中断已被处理
 */
irqreturn_t test_interrupt(int irq,void *args)
{
	printk("This is test_interrupt\n");
	// 将延迟工作添加到工作队列，延迟3秒执行
	queue_delayed_work(test_workqueue,&test_workqueue_work,3*HZ);
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 
 * 模块初始化函数
 * 在模块加载时被调用，完成以下工作：
 * 1. 将GPIO引脚映射到中断号
 * 2. 注册中断处理函数
 * 3. 创建共享工作队列
 * 4. 初始化延迟工作结构体
 * @return: 成功返回0，失败返回负值
 */
static int interrupt_irq_init(void)
{
	int ret;
	irq=gpio_to_irq(101);  // 将GPIO 101映射到中断号

	printk("irq is %d\n",irq);
	
	// 注册中断处理函数，设置为上升沿触发
	ret=request_irq(irq,test_interrupt,IRQF_TRIGGER_RISING,"test",NULL);

	if(ret<0)
	{
		printk("request_irq is error\n");
		return -1;
	}

	// 创建共享工作队列
	test_workqueue=create_workqueue("test_workqueue");

	// 初始化延迟工作结构体，设置处理函数
	INIT_DELAYED_WORK(&test_workqueue_work,test_work);

	return 0;
}

/* 
 * 模块退出函数
 * 在模块卸载时被调用，完成以下工作：
 * 1. 释放中断资源
 * 2. 取消待处理的延迟工作
 * 3. 刷新工作队列
 * 4. 销毁工作队列
 */
static void interrupt_irq_exit(void)
{
	free_irq(irq,NULL);  // 释放中断
	cancel_delayed_work_sync(&test_workqueue_work);  // 取消待处理的延迟工作
	flush_workqueue(test_workqueue);  // 刷新工作队列
	destroy_workqueue(test_workqueue);  // 销毁工作队列
	printk("bye bye\n");
}

// 注册模块的初始化和退出函数
module_init(interrupt_irq_init);
module_exit(interrupt_irq_exit);

// 模块许可证和作者信息
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

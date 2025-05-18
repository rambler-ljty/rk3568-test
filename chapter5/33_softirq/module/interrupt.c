/* 
 * 这是一个使用软中断(softirq)处理中断的驱动程序示例
 * 功能：演示如何在Linux内核中使用软中断机制处理中断
 */

/* 包含必要的内核头文件 */
#include<linux/module.h>    // 模块相关的基本功能
#include<linux/init.h>      // 模块初始化和清理函数
#include<linux/gpio.h>      // GPIO相关功能
#include<linux/interrupt.h> // 中断处理相关功能

//#includ<linux/delay.h>    // 延时相关功能（已注释）

/* 全局变量定义 */
int irq;                    // 中断号

/* 
 * 软中断处理函数
 * 当软中断被触发时，此函数被调用
 * @param softirq_action: 软中断动作结构体指针
 */
void testsoft_func(struct softirq_action *softirq_action)
{
	printk("This is testsoft_func\n");
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
	raise_softirq(TEST_SOFTIRQ);
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 
 * 模块初始化函数
 * 在模块加载时被调用，完成以下工作：
 * 1. 将GPIO引脚映射到中断号
 * 2. 注册中断处理函数
 * 3. 注册软中断处理函数
 * @return: 成功返回0，失败返回负值
 */
static int interrupt_irq_init(void)
{
	int ret;
	irq=gpio_to_irq(101);

	printk("irq is %d\n",irq);

	
	ret=request_irq(irq,test_interrupt,IRQF_TRIGGER_RISING,"test",NULL);

	if(ret<0)
	{
	  printk("request_irq is error\n");
	  return -1;
	
	}

	open_softirq(TEST_SOFTIRQ,testsoft_func);

	return 0;
}

/* 
 * 模块退出函数
 * 在模块卸载时被调用，完成以下工作：
 * 1. 释放中断资源
 */
static void interrupt_irq_exit(void)
{
	free_irq(irq,NULL);
	printk("bye bye\n");
}

module_init(interrupt_irq_init);
module_exit(interrupt_irq_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

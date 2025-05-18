/* 
 * 这是一个使用tasklet处理中断的驱动程序示例
 * 功能：演示如何在Linux内核中使用tasklet机制处理中断
 */

/* 包含必要的内核头文件 */
#include<linux/module.h>    // 模块相关的基本功能
#include<linux/init.h>      // 模块初始化和清理函数
#include<linux/gpio.h>      // GPIO相关功能
#include<linux/interrupt.h> // 中断处理相关功能

//#includ<linux/delay.h>    // 延时相关功能（已注释）

/* 全局变量定义 */
int irq;                    // 中断号
struct tasklet_struct mytasklet;  // 定义tasklet结构体

/* 
 * tasklet处理函数
 * 当tasklet被调度时，此函数被调用
 * @param data: 传递给tasklet的数据
 */
void mytasklet_func(unsigned long data)
{
	printk("data is %ld\n",data);
	//msleep(3000);  // 延时3秒（已注释）
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
	tasklet_schedule(&mytasklet);  // 调度tasklet
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 
 * 模块初始化函数
 * 在模块加载时被调用，完成以下工作：
 * 1. 将GPIO引脚映射到中断号
 * 2. 注册中断处理函数
 * 3. 初始化tasklet
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

	// 初始化tasklet，设置处理函数和传递的数据
	tasklet_init(&mytasklet,mytasklet_func,1);

	return 0;
}

/* 
 * 模块退出函数
 * 在模块卸载时被调用，完成以下工作：
 * 1. 释放中断资源
 * 2. 禁用并清理tasklet
 */
static void interrupt_irq_exit(void)
{
	free_irq(irq,NULL);  // 释放中断
	tasklet_enable(&mytasklet);  // 启用tasklet
	tasklet_kill(&mytasklet);    // 终止tasklet
	printk("bye bye\n");
}

// 注册模块的初始化和退出函数
module_init(interrupt_irq_init);
module_exit(interrupt_irq_exit);

// 模块许可证和作者信息
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

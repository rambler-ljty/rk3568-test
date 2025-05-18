/* 
 * 这是一个使用带数据的工作队列(workqueue with data)处理中断的驱动程序示例
 * 功能：演示如何在Linux内核中使用工作队列机制传递和处理数据
 * 特点：通过自定义结构体将数据与工作队列关联，实现数据传递
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
 * 自定义工作数据结构体
 * 用于将数据与工作队列关联
 */
struct work_data
{
	struct work_struct test_work;  // 工作队列结构体
	int a;                         // 数据成员a
	int b;                         // 数据成员b
};

/* 全局变量定义 */
struct work_data test_workqueue_work;  // 工作数据实例
struct workqueue_struct *test_workqueue;  // 工作队列结构体指针

/* 
 * 工作队列处理函数
 * 当工作被调度时，此函数被调用
 * @param work: 工作队列结构体指针
 */
void test_work(struct work_struct *work)
{
	struct work_data *pdata;
	// 通过container_of宏获取包含work_struct的work_data结构体指针
	pdata=container_of(work,struct work_data,test_work);
	printk("a is %d\n",pdata->a);  // 打印数据成员a
	printk("b is %d\n",pdata->b);  // 打印数据成员b
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
	// 将工作添加到工作队列
	queue_work(test_workqueue,&test_workqueue_work.test_work);
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 
 * 模块初始化函数
 * 在模块加载时被调用，完成以下工作：
 * 1. 将GPIO引脚映射到中断号
 * 2. 注册中断处理函数
 * 3. 创建共享工作队列
 * 4. 初始化工作结构体
 * 5. 设置工作数据
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

	// 初始化工作结构体，设置处理函数
	INIT_WORK(&test_workqueue_work.test_work,test_work);

	// 设置工作数据
	test_workqueue_work.a=1;
	test_workqueue_work.b=2;

	return 0;
}

/* 
 * 模块退出函数
 * 在模块卸载时被调用，完成以下工作：
 * 1. 释放中断资源
 * 2. 取消待处理的工作
 * 3. 刷新工作队列
 * 4. 销毁工作队列
 */
static void interrupt_irq_exit(void)
{
	free_irq(irq,NULL);  // 释放中断
	cancel_work_sync(&test_workqueue_work.test_work);  // 取消待处理的工作
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

/*
 * 这是一个Linux内核模块示例，演示了内核定时器的使用
 * 该模块实现了一个简单的定时器，每5秒触发一次回调函数
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/timer.h>

/* 定时器回调函数声明 */
static void function_test(struct timer_list *t);

/* 定义定时器结构体，并指定回调函数 */
DEFINE_TIMER(timer_test,function_test);

/* 定时器回调函数实现 */
static void function_test(struct timer_list *t)
{
	// 打印调试信息
	printk("this is function test\n");
	// 重新设置定时器，5秒后再次触发
	mod_timer(&timer_test,jiffies_64+msecs_to_jiffies(5000));
}

/* 模块初始化函数 */
static int __init timer_mod_init(void)
{
	// 设置定时器到期时间（5秒后）
	timer_test.expires=jiffies_64+msecs_to_jiffies(5000);
	// 添加定时器到系统
	add_timer(&timer_test);
	return 0;
}

/* 模块退出函数 */
static void __exit timer_mod_exit(void)
{
	// 删除定时器
	del_timer(&timer_test);
	printk("module exit\n");
}

// 模块入口和出口
module_init(timer_mod_init);
module_exit(timer_mod_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

/*
 * myinput_dev.c - Linux内核输入设备驱动模块（带定时器版本）
 * 
 * 这个模块实现了一个带定时器的输入设备驱动，用于演示Linux输入子系统的高级用法。
 * 该驱动创建一个虚拟的输入设备，通过定时器自动产生按键事件。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/input.h>     // 提供输入设备相关的功能
#include<linux/init.h>      // 提供初始化相关的功能
#include<linux/timer.h>     // 提供定时器相关的功能

// 全局变量：指向输入设备结构体的指针
struct input_dev *myinput_dev;

// 定时器回调函数声明
static void timer_function(struct timer_list *t);

// 定义定时器结构体
DEFINE_TIMER(test_timer,timer_function);

/*
 * timer_function - 定时器回调函数
 * 
 * 功能：
 * 1. 模拟按键按下和释放事件
 * 2. 每秒切换一次按键状态（按下/释放）
 * 3. 重新设置定时器，实现周期性触发
 * 
 * 参数：
 * t - 定时器结构体指针
 */
static void timer_function(struct timer_list *t){
	static int value=0;  // 静态变量，用于切换按键状态
	value=value?0:1;     // 切换状态：0表示释放，1表示按下

	// 发送按键事件
	input_event(myinput_dev,EV_KEY,KEY_1,value);
	// 发送同步事件，表示事件序列结束
	input_event(myinput_dev,EV_SYN,SYN_REPORT,0);

	// 重新设置定时器，1秒后再次触发
	mod_timer(&test_timer,jiffies+msecs_to_jiffies(1000));
}

/*
 * myinput_dev_init - 模块初始化函数
 * 
 * 功能：
 * 1. 分配输入设备结构体
 * 2. 设置设备名称和事件类型
 * 3. 注册输入设备到内核
 * 4. 启动定时器，开始自动产生按键事件
 * 
 * 返回值：
 * 0 - 成功
 * 负数 - 失败
 */
static int myinput_dev_init(void)
{
	int ret;
	
	// 分配输入设备结构体
	myinput_dev=input_allocate_device();
	if(myinput_dev==NULL){
		printk("input_allocate_device error\n");
		return -1;
	}

	// 设置设备名称
	myinput_dev->name="myinput_dev";
	
	// 设置支持的事件类型：按键事件和同步事件
	set_bit(EV_KEY,myinput_dev->evbit);
	set_bit(EV_SYN,myinput_dev->evbit);
	
	// 设置支持的按键：数字键1
	set_bit(KEY_1,myinput_dev->keybit);
	
	// 注册输入设备到内核
	ret=input_register_device(myinput_dev);
	if(ret<0){
		printk("input_register_device error\n");
		goto error;
	}
	
	// 启动定时器，1秒后开始产生按键事件
	mod_timer(&test_timer,jiffies+msecs_to_jiffies(1000));

	return 0;

error:
	// 错误处理：释放已分配的输入设备
	input_free_device(myinput_dev);
	return ret;
}

/*
 * myinput_dev_exit - 模块退出函数
 * 
 * 功能：
 * 1. 删除定时器，停止自动产生事件
 * 2. 注销输入设备
 * 3. 释放相关资源
 */
static void myinput_dev_exit(void)
{
	// 删除定时器
	del_timer(&test_timer);
	// 注销输入设备
	input_unregister_device(myinput_dev);
	// 释放输入设备
	input_free_device(myinput_dev);
}

// 模块初始化和退出函数注册
module_init(myinput_dev_init);
module_exit(myinput_dev_exit);

// 模块信息
MODULE_LICENSE("GPL");   // 模块许可证
MODULE_AUTHOR("topeet");  


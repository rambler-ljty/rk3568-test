#include<linux/module.h>    // 提供模块相关的功能
#include<linux/input.h>     // 提供输入设备相关的功能

// 全局变量：指向输入设备结构体的指针
struct input_dev *myinput_dev;

/*
 * myinput_dev_init - 模块初始化函数
 * 
 * 功能：
 * 1. 分配输入设备结构体
 * 2. 设置设备名称和事件类型
 * 3. 注册输入设备到内核
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
	
	// 设置支持的事件类型：按键事件
	__set_bit(EV_KEY,myinput_dev->evbit);
	
	// 设置支持的按键：数字键1
	__set_bit(KEY_1,myinput_dev->keybit);
	
	// 注册输入设备到内核
	ret=input_register_device(myinput_dev);
	if(ret<0){
		printk("input_register_device error\n");
		goto error;
	}

	return 0;

error:
	// 错误处理：释放已分配的输入设备
	input_free_device(myinput_dev);
	return ret;
}

/*
 * myinput_dev_exit - 模块退出函数
 */
static void myinput_dev_exit(void)
{
	// 注销输入设备
	input_unregister_device(myinput_dev);
}

// 模块初始化和退出函数注册
module_init(myinput_dev_init);
module_exit(myinput_dev_exit);

// 模块信息
MODULE_LICENSE("GPL");   // 模块许可证
MODULE_AUTHOR("topeet");  


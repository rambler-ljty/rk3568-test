/*
 * FT5X06触摸屏I2C驱动模块 (支持输入子系统)
 * 
 * 本模块实现了一个完整的FT5X06触摸屏控制器I2C驱动
 * 支持GPIO控制、中断处理、输入子系统和触摸坐标上报功能
 * 
 * 功能特性:
 * - I2C通信接口
 * - GPIO复位控制
 * - 中断处理机制
 * - 寄存器读写操作
 * - 输入子系统集成
 * - 触摸坐标上报
 * - 工作队列处理
 * 
 */

#include <linux/init.h>           // 模块初始化和退出宏定义
#include <linux/module.h>         // 内核模块基本功能
#include <linux/i2c.h>            // I2C总线驱动接口
#include <linux/gpio.h>           // GPIO基本功能
#include <linux/gpio/consumer.h>  // GPIO描述符接口
#include <linux/interrupt.h>      // 中断处理接口
#include <linux/delay.h>          // 延时函数
#include <linux/input.h>          // 输入子系统接口
#include <linux/workqueue.h>      // 工作队列接口

// 工作队列处理函数声明
void ft5x06_func(struct work_struct *work);

// 声明工作队列，用于在中断上下文中调度触摸处理
static DECLARE_WORK(ft5x06_work,ft5x06_func);

// 全局GPIO描述符指针
struct gpio_desc *reset_gpio, *irq_gpio;

// 全局I2C客户端指针，用于寄存器操作
struct i2c_client *ft5x06_client;

// 输入设备指针，用于向用户空间上报触摸事件
struct input_dev *ft5x06_input_dev;

/**
 * ft5x06_read_reg - 读取FT5X06寄存器
 * @reg_addr: 要读取的寄存器地址
 * 
 * 通过I2C总线读取指定寄存器的值
 * 使用两个I2C消息：先发送寄存器地址，再读取数据
 * 
 * 返回值: 成功返回寄存器值，失败返回负错误码
 */
int ft5x06_read_reg(u8 reg_addr)
{
	u8 data;

	// 构造I2C消息数组
	struct i2c_msg msgs[2] ={
		[0]={
			.addr=ft5x06_client->addr,    // I2C设备地址
			.flags=0,                     // 写标志
			.len=sizeof(reg_addr),        // 寄存器地址长度
			.buf=&reg_addr,               // 寄存器地址缓冲区
		},
		[1]={
			.addr=ft5x06_client->addr,    // I2C设备地址
			.flags=I2C_M_RD,              // 读标志
			.len=sizeof(data),            // 数据长度
			.buf=&data,                   // 数据缓冲区
		},
	};

	// 执行I2C传输
	if(i2c_transfer(ft5x06_client->adapter,msgs,ARRAY_SIZE(msgs))!=ARRAY_SIZE(msgs)){
		return -EIO;
	}

	return data;
}

/**
 * ft5x06_write_reg - 写入FT5X06寄存器
 * @reg_addr: 要写入的寄存器地址
 * @data: 要写入的数据缓冲区
 * @len: 数据长度
 * 
 * 通过I2C总线向指定寄存器写入数据
 * 将寄存器地址和数据组合成一个I2C消息发送
 */
void ft5x06_write_reg(u8 reg_addr, u8 *data, u16 len){
	u8 buff[256];

	// 构造I2C消息
	struct i2c_msg msgs[]={
		[0]={
			.addr=ft5x06_client->addr,    // I2C设备地址
			.flags=0,                     // 写标志
			.len=len+1,                   // 总长度（地址+数据）
			.buf=buff,                    // 数据缓冲区
		},
	};

	// 组合寄存器地址和数据
	buff[0]=reg_addr;                    // 第一个字节是寄存器地址
	memcpy(&buff[1],data,len);           // 复制数据到缓冲区

	// 执行I2C传输
	if(i2c_transfer(ft5x06_client->adapter,msgs,ARRAY_SIZE(msgs))!=ARRAY_SIZE(msgs)){
		return;
	}
}


/**
 * ft5x06_func - 触摸事件处理工作队列函数
 * @work: 工作队列结构体指针
 * 
 * 在工作队列上下文中处理触摸事件
 * 读取触摸坐标和状态，并通过输入子系统上报给用户空间
 * 
 * 处理流程:
 * 1. 读取X坐标高字节和低字节
 * 2. 读取Y坐标高字节和低字节
 * 3. 读取触摸状态
 * 4. 根据状态上报触摸事件
 */
void ft5x06_func(struct work_struct *work)
{
	int TOUCH1_XH,TOUCH1_XL,x;
	int TOUCH1_YH,TOUCH1_YL,y;
	int TD_STATUS;

	// 读取X坐标（16位，分为高低字节）
	TOUCH1_XH=ft5x06_read_reg(0x03);  // X坐标高字节
	TOUCH1_XL=ft5x06_read_reg(0x04);  // X坐标低字节
	x=((TOUCH1_XH <<8)|TOUCH1_XL) & 0xfff;  // 组合成12位坐标
	
	// 读取Y坐标（16位，分为高低字节）
	TOUCH1_YH=ft5x06_read_reg(0x05);  // Y坐标高字节
	TOUCH1_YL=ft5x06_read_reg(0x06);  // Y坐标低字节
	y=((TOUCH1_YH <<8)|TOUCH1_YL) &0xfff;   // 组合成12位坐标

	// 读取触摸状态寄存器
	TD_STATUS=ft5x06_read_reg(0x02);
	TD_STATUS=TD_STATUS & 0xf;  // 取低4位作为触摸状态

	// 根据触摸状态上报事件
	if(TD_STATUS == 0){
		// 无触摸，上报按键释放事件
		input_report_key(ft5x06_input_dev,BTN_TOUCH,0);
		input_sync(ft5x06_input_dev);
	}else{
		// 有触摸，上报按键按下和坐标事件
		input_report_key(ft5x06_input_dev,BTN_TOUCH,1);
		input_report_abs(ft5x06_input_dev,ABS_X,x);  // 上报X坐标
		input_report_abs(ft5x06_input_dev,ABS_Y,y);  // 上报Y坐标（注意：原代码有错误，应该是ABS_Y）
		input_sync(ft5x06_input_dev);  // 同步事件
	}
}



/**
 * ft5x06_handler - FT5X06中断处理函数
 * @irq: 中断号
 * @args: 中断参数
 * 
 * 当触摸屏产生中断时调用此函数
 * 将触摸处理任务调度到工作队列中执行
 * 
 * 返回值: IRQ_RETVAL(IRQ_HANDLED) 表示中断已处理
 */
irqreturn_t ft5x06_handler(int irq,void *args)
{
	// 将触摸处理任务调度到工作队列
	schedule_work(&ft5x06_work);
	return IRQ_RETVAL(IRQ_HANDLED);
}



/**
 * ft5x06_probe - FT5X06设备探测和初始化函数
 * @client: I2C客户端结构体指针
 * @id: 设备ID结构体指针
 * 
 * 当内核检测到匹配的FT5X06设备时调用此函数
 * 负责：
 * 1. 保存I2C客户端指针
 * 2. 获取GPIO资源（复位和中断）
 * 3. 执行硬件复位序列
 * 4. 注册中断处理函数
 * 5. 创建和配置输入设备
 * 6. 注册输入设备到输入子系统
 * 
 * 返回值: 成功返回0，失败返回负错误码
 */
int ft5x06_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	printk("This is ft5x06 probe\n");
	
	// 保存I2C客户端指针，供寄存器操作使用
	ft5x06_client=client;
	
	// 获取复位GPIO描述符
	reset_gpio=gpiod_get_optional(&client->dev,"reset",0);
	if(!reset_gpio){
		printk("gpiod_get_optional reset gpio is error\n");
		return -1;
	}

	// 获取中断GPIO描述符
	irq_gpio=gpiod_get_optional(&client->dev,"interrupts",0);
	if(!irq_gpio)
	{
		printk("gpiod_get_optional irq gpio is error");
		return -1;
	}
	
	// 执行硬件复位序列
	gpiod_direction_output(reset_gpio,0);  // 拉低复位引脚
	msleep(5);                             // 延时5ms
	gpiod_direction_output(reset_gpio,1);  // 拉高复位引脚，完成复位

	// 注册中断处理函数
	ret=request_irq(client->irq,ft5x06_handler,IRQ_TYPE_EDGE_FALLING | IRQF_ONESHOT, "ft5x06 irq", NULL);
	if(ret<0)
	{
		printk("request irq is error\n");
		return -2;
	}
	
	// 分配输入设备
	ft5x06_input_dev=input_allocate_device();
	if(ft5x06_input_dev==NULL){
		printk("input allocate device is error\n");
		return -2;
	}

	// 设置输入设备名称
	ft5x06_input_dev->name ="ft5x06_dev";

	// 设置支持的事件类型
	set_bit(EV_KEY,ft5x06_input_dev->evbit);    // 支持按键事件
	set_bit(BTN_TOUCH,ft5x06_input_dev->keybit); // 支持触摸按键
	set_bit(EV_ABS,ft5x06_input_dev->evbit);    // 支持绝对坐标事件
	set_bit(ABS_X,ft5x06_input_dev->absbit);    // 支持X坐标
	set_bit(ABS_Y,ft5x06_input_dev->absbit);    // 支持Y坐标

	// 设置绝对坐标参数（X: 0-800, Y: 0-1280）
	input_set_abs_params(ft5x06_input_dev,ABS_X,0,800,0,0);
	input_set_abs_params(ft5x06_input_dev,ABS_Y,0,1280,0,0);

	// 注册输入设备到输入子系统
	ret=input_register_device(ft5x06_input_dev);
	if(ret<0){
		printk("input register device error\n");
		goto error_0;
	}
	
	return 0;

// 错误处理：清理已分配的资源
error_0:
	input_free_device(ft5x06_input_dev);
	free_irq(client->irq,NULL);
	gpiod_put(reset_gpio);
	gpiod_put(irq_gpio);

	return ret;
}


/**
 * ft5x06_remove - FT5X06设备移除函数
 * @client: I2C客户端结构体指针
 * 
 * 当设备被移除或模块卸载时调用此函数
 * 负责：
 * 1. 释放中断资源
 * 2. 释放GPIO资源
 * 3. 清理相关内存
 * 
 * 返回值: 成功返回0
 */
int ft5x06_remove(struct i2c_client *client){
	// 释放中断资源
	free_irq(client->irq,NULL);
	
	// 释放GPIO资源
	gpiod_put(reset_gpio);
	gpiod_put(irq_gpio);

	return 0;
}

/**
 * ft5x06_id - I2C设备ID匹配表
 * 
 * 定义支持的设备类型和对应的私有数据
 * 用于内核通过设备ID识别和绑定对应的驱动
 */
static const struct i2c_device_id ft5x06_id[]={
	{"my-ft5x06",0},  // 设备类型名称和私有数据
	{},               // 空条目表示匹配表结束
};

/**
 * ft5x06_driver - I2C驱动结构体
 * 
 * 定义I2C驱动的各种回调函数和属性
 * 包括设备ID匹配、probe/remove函数等
 */
static struct i2c_driver ft5x06_driver = {
	.driver = {
		.owner= THIS_MODULE,           // 模块所有者
		.name="my-ft5x06",             // 驱动名称
	},
	.probe=ft5x06_probe,              // 设备探测函数
	.remove=ft5x06_remove,            // 设备移除函数
	.id_table=ft5x06_id,              // 设备ID匹配表
};

/**
 * ft5x06_driver_init - 驱动模块初始化函数
 * 
 * 模块加载时调用，负责向I2C子系统注册驱动
 * 注册后内核会自动扫描并匹配相应的设备
 * 
 * 返回值: 成功返回0，失败返回负错误码
 */
static int __init ft5x06_driver_init(void){
	int ret;

	// 向I2C子系统注册驱动
	ret=i2c_add_driver(&ft5x06_driver);
	if(ret<0){
		printk("i2c_add_driver is error\n");
		return ret;
	}
	return 0;
}

/**
 * ft5x06_driver_exit - 驱动模块退出函数
 * 
 * 模块卸载时调用，负责：
 * 1. 释放中断资源
 * 2. 注销输入设备
 * 3. 释放GPIO资源
 * 4. 从I2C子系统注销驱动
 */
static void __exit ft5x06_driver_exit(void){
	// 释放中断资源
	free_irq(ft5x06_client->irq,NULL);

	// 注销输入设备
	input_unregister_device(ft5x06_input_dev);

	// 释放GPIO资源
	gpiod_put(reset_gpio);
	gpiod_put(irq_gpio);

	// 从I2C子系统注销驱动
	i2c_del_driver(&ft5x06_driver);
}

// 指定模块的初始化和退出函数
module_init(ft5x06_driver_init);
module_exit(ft5x06_driver_exit);

// 指定模块许可证
MODULE_LICENSE("GPL");

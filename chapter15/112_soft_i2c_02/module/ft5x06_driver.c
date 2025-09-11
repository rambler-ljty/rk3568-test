/**
 * @file ft5x06_driver.c
 * @brief FT5X06触摸屏I2C驱动模块
 * 
 * 本文件实现了基于i2c_device_id匹配的FT5X06触摸屏I2C驱动。
 * 使用硬件I2C控制器进行通信，支持寄存器读写操作。
 * 
 * 功能特性:
 * - 基于i2c_device_id的设备匹配
 * - 寄存器读写操作
 * - 硬件I2C通信
 * - 模块化设计
 * 
 */

#include <linux/init.h>        // 模块初始化和退出宏定义
#include <linux/module.h>      // 内核模块基本功能
#include <linux/i2c.h>         // I2C子系统接口
#include <linux/gpio.h>        // GPIO控制接口
#include <linux/gpio/consumer.h> // GPIO消费者接口
#include <linux/interrupt.h>   // 中断处理接口
#include <linux/delay.h>       // 延时函数

// 全局I2C客户端指针，用于存储匹配到的I2C设备
struct i2c_client *ft5x06_client;

/**
 * @brief 读取FT5X06寄存器
 * 
 * 通过I2C总线读取指定寄存器的数据。使用两个I2C消息：
 * 第一个消息发送寄存器地址，第二个消息读取数据。
 * 
 * @param reg_addr 要读取的寄存器地址
 * @return 成功返回读取的数据，失败返回-EIO
 */
int ft5x06_read_reg(u8 reg_addr)
{
	u8 data;  // 存储读取的数据

	// 构造I2C消息数组，包含写地址和读数据两个消息
	struct i2c_msg msgs[2] ={
		[0]={  // 第一个消息：发送寄存器地址
			.addr=ft5x06_client->addr,  // I2C设备地址
			.flags=0,                   // 写操作标志
			.len=sizeof(reg_addr),      // 数据长度
			.buf=&reg_addr,             // 数据缓冲区
		},
		[1]={  // 第二个消息：读取寄存器数据
			.addr=ft5x06_client->addr,  // I2C设备地址
			.flags=I2C_M_RD,            // 读操作标志
			.len=sizeof(data),          // 数据长度
			.buf=&data,                 // 数据缓冲区
		},
	};

	// 执行I2C传输，检查是否成功传输了所有消息
	if(i2c_transfer(ft5x06_client->adapter,msgs,ARRAY_SIZE(msgs))!=ARRAY_SIZE(msgs)){
		return -EIO;  // 传输失败，返回I/O错误
	}

	return data;  // 返回读取的数据
}


/**
 * @brief 写入FT5X06寄存器
 * 
 * 通过I2C总线向指定寄存器写入数据。将寄存器地址和数据
 * 组合成一个消息发送。
 * 
 * @param reg_addr 要写入的寄存器地址
 * @param data 要写入的数据指针
 * @param len 数据长度
 */
void ft5x06_write_reg(u8 reg_addr, u8 *data, u16 len){
	u8 buff[256];  // 临时缓冲区，用于组合寄存器地址和数据

	// 构造I2C写消息
	struct i2c_msg msgs[]={
		[0]={
			.addr=ft5x06_client->addr,  // I2C设备地址
			.flags=0,                   // 写操作标志
			.len=len+1,                 // 数据长度（寄存器地址+数据）
			.buf=buff,                  // 数据缓冲区
		},
	};

	// 将寄存器地址放在缓冲区第一个字节
	buff[0]=reg_addr;
	// 将数据复制到缓冲区后续位置
	memcpy(&buff[1],data,len);

	// 执行I2C传输
	if(i2c_transfer(ft5x06_client->adapter,msgs,ARRAY_SIZE(msgs))!=ARRAY_SIZE(msgs)){
		return;  // 传输失败，直接返回
	}
}


/**
 * @brief FT5X06驱动探测函数
 * 
 * 当I2C设备匹配成功时调用此函数。执行设备初始化、
 * 寄存器读写测试等操作。
 * 
 * @param client 匹配到的I2C客户端
 * @param id 匹配的设备ID
 * @return 成功返回0，失败返回负错误码
 */
int ft5x06_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int value;  // 存储读取的寄存器值
	printk("This is ft5x06 probe\n");
	
	// 保存I2C客户端指针，供其他函数使用
	ft5x06_client=client;
	
	// 向寄存器0x80写入测试数据0x4b
	ft5x06_write_reg(0x80,&(u8){0x4b},1);
	
	// 读取寄存器0x80的值进行验证
	value=ft5x06_read_reg(0x80);
	printk("reg 0x80 is %x\n",value);

	return 0;  // 探测成功
}


/**
 * @brief FT5X06驱动移除函数
 * 
 * 当I2C设备被移除时调用此函数。执行清理操作。
 * 
 * @param client 要移除的I2C客户端
 * @return 成功返回0
 */
int ft5x06_remove(struct i2c_client *client){
	// 清理资源（当前无需特殊清理）
	return 0;
}


// I2C设备ID表，用于设备匹配
static const struct i2c_device_id ft5x06_id[]={
	{"my-ft5x06",0},  // 设备名称和ID
	{},               // 结束标记
};


// I2C驱动结构体，定义驱动的基本信息和回调函数
static struct i2c_driver ft5x06_driver = {
	.driver = {
		.owner= THIS_MODULE,    // 模块所有者
		.name="my-ft5x06",      // 驱动名称
	},
	.probe=ft5x06_probe,        // 设备探测函数
	.remove=ft5x06_remove,      // 设备移除函数
	.id_table=ft5x06_id,        // 设备ID表
};


/**
 * @brief 驱动模块初始化函数
 * 
 * 模块加载时调用，注册I2C驱动到内核。
 * 
 * @return 成功返回0，失败返回负错误码
 */
static int __init ft5x06_driver_init(void){
	int ret;

	// 注册I2C驱动到内核
	ret=i2c_add_driver(&ft5x06_driver);
	if(ret<0){
		printk("i2c_add_driver is error\n");
		return ret;
	}
	return 0;  // 初始化成功
}


/**
 * @brief 驱动模块退出函数
 * 
 * 模块卸载时调用，从内核中注销I2C驱动。
 */
static void __exit ft5x06_driver_exit(void){
	// 从内核中注销I2C驱动
	i2c_del_driver(&ft5x06_driver);
}

// 模块初始化和退出函数声明
module_init(ft5x06_driver_init);
module_exit(ft5x06_driver_exit);
MODULE_LICENSE("GPL");  // 模块许可证

/*
 * FT5X06触摸屏软件I2C驱动模块
 * 
 * 本模块实现了一个基于GPIO的软件I2C驱动
 * 通过GPIO模拟I2C协议与FT5X06触摸屏控制器通信
 * 
 * 功能特性:
 * - 软件I2C协议实现
 * - GPIO模拟SCL和SDA信号
 * - 完整的I2C时序控制
 * - 寄存器读写操作
 * - 硬件功能测试
 * 
 */

#include <linux/init.h>           // 模块初始化和退出宏定义
#include <linux/module.h>         // 内核模块基本功能
#include <linux/gpio/consumer.h>  // GPIO描述符接口
#include <linux/gpio.h>           // GPIO基本功能
#include <linux/delay.h>          // 延时函数
#include <linux/jiffies.h>        // 时间相关函数

// I2C引脚定义
#define I2C_SCL 11  // I2C时钟线GPIO引脚号
#define I2C_SDA 12  // I2C数据线GPIO引脚号

// GPIO描述符指针
struct gpio_desc *i2c_scl_desc;  // SCL引脚描述符
struct gpio_desc *i2c_sda_desc;  // SDA引脚描述符

/**
 * i2c_start - 产生I2C起始信号
 * 
 * 按照I2C协议产生起始条件：
 * 1. SCL和SDA都为高电平
 * 2. SDA从高电平变为低电平（在SCL为高电平时）
 * 3. SCL变为低电平
 */
void i2c_start(void){
	// 步骤1: 确保SCL和SDA都为高电平
	gpiod_direction_output(i2c_scl_desc,1);
	gpiod_direction_output(i2c_sda_desc,1);
	mdelay(1);

	// 步骤2: SDA从高电平变为低电平（在SCL为高电平时）
	gpiod_direction_output(i2c_sda_desc,0);
	mdelay(1);

	// 步骤3: SCL变为低电平
	gpiod_direction_output(i2c_scl_desc,0);
	mdelay(1);
}

/**
 * i2c_stop - 产生I2C停止信号
 * 
 * 按照I2C协议产生停止条件：
 * 1. SCL和SDA都为低电平
 * 2. SCL变为高电平
 * 3. SDA从低电平变为高电平（在SCL为高电平时）
 */
void i2c_stop(void){
	// 步骤1: 确保SCL和SDA都为低电平
	gpiod_direction_output(i2c_scl_desc,0);
	gpiod_direction_output(i2c_sda_desc,0);
	mdelay(1);

	// 步骤2: SCL变为高电平
	gpiod_direction_output(i2c_sda_desc,1);
	mdelay(1);

	// 步骤3: SDA从低电平变为高电平（在SCL为高电平时）
	gpiod_direction_output(i2c_scl_desc,1);
	mdelay(1);
}

/**
 * i2c_send_ack - 发送I2C应答信号
 * @ack: 应答信号，0表示ACK，1表示NACK
 * 
 * 按照I2C协议发送应答信号：
 * 1. 设置SDA为输出模式
 * 2. 根据ack参数设置SDA电平
 * 3. 产生SCL时钟脉冲
 */
void i2c_send_ack(int ack){
	// 设置SDA为输出模式
	gpiod_direction_output(i2c_sda_desc,0);

	// 根据ack参数设置SDA电平
	if(ack){
		gpiod_direction_output(i2c_sda_desc,0);  // ACK: 低电平
	}else{
		gpiod_direction_output(i2c_sda_desc,1);  // NACK: 高电平
	}

	// 产生SCL时钟脉冲
	gpiod_direction_output(i2c_scl_desc,1);
	mdelay(1);
	gpiod_direction_output(i2c_scl_desc,0);
}

/**
 * i2c_recv_ack - 接收I2C应答信号
 * 
 * 按照I2C协议接收应答信号：
 * 1. 设置SDA为输入模式
 * 2. 产生SCL时钟脉冲
 * 3. 读取SDA电平状态
 * 4. 恢复SDA为输出模式
 * 
 * 返回值: 0表示ACK，1表示NACK
 */
int i2c_recv_ack(void){
	int value=0;

	// 设置SDA为输入模式
	gpiod_direction_input(i2c_sda_desc);

	// 产生SCL时钟脉冲
	gpiod_direction_output(i2c_scl_desc,1);
	mdelay(1);
	
	// 读取SDA电平状态
	if(gpiod_get_value(i2c_sda_desc)){
		value=1;  // NACK: 高电平
	}else{
		value=0;  // ACK: 低电平
	}

	// 恢复SCL为低电平
	gpiod_direction_output(i2c_scl_desc,0);

	// 恢复SDA为输出模式
	gpiod_direction_output(i2c_sda_desc,1);

	return value;
}

/**
 * i2c_send_data - 发送I2C数据
 * @data: 要发送的8位数据
 * 
 * 按照I2C协议发送8位数据：
 * 1. 从最高位开始发送
 * 2. 每个位在SCL低电平时设置SDA
 * 3. 在SCL高电平时保持SDA稳定
 */
void i2c_send_data(int data){
	int i;
	int value;

	// 确保SCL为低电平
	gpiod_direction_output(i2c_scl_desc,0);

	// 从最高位开始发送8位数据
	for(i=0;i<8;i++){
		// 提取当前位（从最高位开始）
		value=(data<<i)&0x80;
		
		// 设置SDA电平
		if(value){
			gpiod_direction_output(i2c_sda_desc,1);  // 发送1
		}else{
			gpiod_direction_output(i2c_sda_desc,0);  // 发送0
		}
	
		// 产生SCL时钟脉冲
		gpiod_direction_output(i2c_scl_desc,1);
		mdelay(1);
		gpiod_direction_output(i2c_scl_desc,0);
		mdelay(1);
	}
}

/**
 * i2c_recv_data - 接收I2C数据
 * 
 * 按照I2C协议接收8位数据：
 * 1. 设置SDA为输入模式
 * 2. 从最高位开始接收
 * 3. 每个位在SCL高电平时读取SDA
 * 4. 恢复SDA为输出模式
 * 
 * 返回值: 接收到的8位数据
 */
int i2c_recv_data(void){
	int i;
	int temp=0;
	int data=0;

	// 设置SDA为输入模式
	gpiod_direction_input(i2c_sda_desc);
	mdelay(1);

	// 从最高位开始接收8位数据
	for(i=0;i<8;i++){
		// 确保SCL为低电平
		gpiod_direction_output(i2c_scl_desc,0);
		mdelay(1);

		// 在SCL高电平时读取SDA
		gpiod_direction_output(i2c_scl_desc,1);
		mdelay(1);

		// 读取SDA电平
		data=gpiod_get_value(i2c_sda_desc);

		// 将接收到的位组合成数据
		if(data){
			temp=(temp<<1)|data;  // 接收1
		}else{
			temp=(temp<<1)&~data; // 接收0
		}
	}

	// 恢复SCL为低电平
	gpiod_direction_output(i2c_scl_desc,0);
	mdelay(1);

	// 恢复SDA为输出模式
	gpiod_direction_output(i2c_sda_desc,1);

	return temp;
}


/**
 * ft5x06_write_reg - 写入FT5X06寄存器
 * @addr: I2C设备地址
 * @reg: 要写入的寄存器地址
 * @value: 要写入的数据
 * 
 * 通过软件I2C向FT5X06写入寄存器数据
 * 按照I2C写操作协议：
 * 1. 发送起始信号
 * 2. 发送设备地址（写模式）
 * 3. 发送寄存器地址
 * 4. 发送数据
 * 5. 发送停止信号
 */
void ft5x06_write_reg(int addr,int reg, int value)
{
	int ack;

	// 发送起始信号
	i2c_start();

	// 发送设备地址（写模式：最低位为0）
	i2c_send_data(addr<<1|0x00);
	ack=i2c_recv_ack();
	if(ack){
		printk("send write+addr error\n");
		goto end;
	}

	// 发送寄存器地址
	i2c_send_data(reg);
	ack=i2c_recv_ack();
	if(ack){
		printk("send reg error\n");
		goto end;
	}

	// 发送数据
	i2c_send_data(value);
	ack=i2c_recv_ack();
	if(ack){
		printk("send value error\n");
	}

end:
	// 发送停止信号
	i2c_stop();
}

/**
 * ft5x06_read_reg - 读取FT5X06寄存器
 * @addr: I2C设备地址
 * @reg: 要读取的寄存器地址
 * 
 * 通过软件I2C从FT5X06读取寄存器数据
 * 按照I2C读操作协议：
 * 1. 发送起始信号
 * 2. 发送设备地址（写模式）
 * 3. 发送寄存器地址
 * 4. 发送重复起始信号
 * 5. 发送设备地址（读模式）
 * 6. 读取数据
 * 7. 发送停止信号
 * 
 * 返回值: 读取到的寄存器数据
 */
int ft5x06_read_reg(int addr,int reg){
	int ack;
	int data;

	// 发送起始信号
	i2c_start();

	// 发送设备地址（写模式：最低位为0）
	i2c_send_data(addr<<1 | 0x00);
	ack=i2c_recv_ack();
	if(ack){
		printk("send write+addr error\n");
		goto end;
	}

	// 发送寄存器地址
	i2c_send_data(reg);
	ack=i2c_recv_ack();
	if(ack){
		printk("send reg error\n");
		goto end;
	}

	// 发送重复起始信号
	i2c_start();
	
	// 发送设备地址（读模式：最低位为1）
	i2c_send_data(addr<<1|0x01);
	ack=i2c_recv_ack();
	if(ack){
		printk("send read+addr error");
		goto end;
	}

	// 读取数据
	data=i2c_recv_data();
	printk("data is %d\n",data);

	// 发送NACK信号（表示不再读取）
	i2c_send_ack(0);

end:
	// 发送停止信号
	i2c_stop();

	return data;
}


/**
 * ft5x06_driver_init - 驱动模块初始化函数
 * 
 * 模块加载时调用，负责：
 * 1. 获取SCL和SDA GPIO描述符
 * 2. 初始化GPIO为输出模式
 * 3. 测试软件I2C通信功能
 * 
 * 返回值: 成功返回0，失败返回负错误码
 */
static int ft5x06_driver_init(void)
{
	// 获取SCL GPIO描述符
	i2c_scl_desc=gpio_to_desc(I2C_SCL);
	if(i2c_scl_desc==NULL){
		printk("gpio_to_desc_error for SCL pin\n");
		return -1;
	}

	// 获取SDA GPIO描述符
	i2c_sda_desc=gpio_to_desc(I2C_SDA);
	if(i2c_sda_desc==NULL){
		printk("gpio_to_desc_error for SDA pin\n");
		return -1;
	}

	// 初始化GPIO为输出模式，设置为高电平（空闲状态）
	gpiod_direction_output(i2c_scl_desc,1);
	gpiod_direction_output(i2c_sda_desc,1);

	// 测试软件I2C通信功能
	ft5x06_write_reg(0x38,0x80,0x33);  // 向寄存器0x80写入0x33
	ft5x06_read_reg(0x38,0x80);        // 从寄存器0x80读取数据

	return 0;
}

/**
 * ft5x06_driver_exit - 驱动模块退出函数
 * 
 * 模块卸载时调用，负责：
 * 1. 释放SCL GPIO资源
 * 2. 释放SDA GPIO资源
 * 3. 清理相关资源
 */
static void ft5x06_driver_exit(void)
{
	// 释放GPIO资源
	gpiod_put(i2c_scl_desc);
	gpiod_put(i2c_sda_desc);
}

// 指定模块的初始化和退出函数
module_init(ft5x06_driver_init);
module_exit(ft5x06_driver_exit);

// 指定模块许可证
MODULE_LICENSE("GPL");


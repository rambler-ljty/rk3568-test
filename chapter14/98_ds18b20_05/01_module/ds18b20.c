/*
 * DS18B20温度传感器Linux内核驱动模块 - 温度读取应用版本
 * 功能：实现DS18B20温度传感器的完整应用，集成GPIO控制、通信协议和温度读取功能
 * 包含DS18B20复位时序、读写操作、温度转换、温度读取等完整功能
 */

#include <linux/init.h>        // 模块初始化和退出宏定义
#include <linux/module.h>      // 内核模块相关宏定义
#include <linux/platform_device.h>  // 平台设备驱动框架
#include <linux/of.h>          // 设备树支持
#include <linux/fs.h>          // 文件系统相关定义
#include <linux/cdev.h>        // 字符设备相关定义
#include <linux/kdev_t.h>      // 设备号相关定义
#include <linux/slab.h>        // 内存分配函数
#include <linux/gpio.h>        // GPIO相关定义
#include <linux/gpio/consumer.h>  // GPIO消费者接口
#include <linux/delay.h>       // 延时函数
#include <linux/uaccess.h>     // 用户空间访问函数（新增）

/**
 * ds18b20_data - DS18B20设备数据结构体
 * 包含字符设备所需的所有信息和GPIO控制
 */
struct ds18b20_data
{
	dev_t dev_num;                    // 设备号
	struct cdev ds18b20_cdev;         // 字符设备结构体
	struct class *ds18b20_class;      // 设备类指针
	struct device *ds18b20_device;    // 设备指针
	struct gpio_desc *ds18b20_gpio;   // GPIO描述符指针，用于控制DS18B20的数据线
};

// 全局设备数据结构体指针
struct ds18b20_data *ds18b20;

/**
 * ds18b20_reset - DS18B20复位函数
 * 功能：实现DS18B20的复位时序，这是1-Wire通信协议的第一步
 * 
 * DS18B20复位时序：
 * 1. 主机拉低数据线至少480us（复位脉冲）
 * 2. 主机释放数据线，上拉电阻将数据线拉高
 * 3. DS18B20等待15-60us后，拉低数据线60-240us（存在脉冲）
 * 4. 主机检测到存在脉冲后，等待复位完成
 */
void ds18b20_reset(void)
{
	// 1. 设置GPIO为输出模式，输出高电平
	gpiod_direction_output(ds18b20->ds18b20_gpio,1);
	
	// 2. 拉低数据线，开始复位脉冲
	gpiod_set_value(ds18b20->ds18b20_gpio,0);
	
	// 3. 保持低电平700us，确保复位脉冲足够长
	udelay(700);

	// 4. 释放数据线，输出高电平
	gpiod_set_value(ds18b20->ds18b20_gpio,1);
	
	// 5. 切换GPIO为输入模式，准备检测DS18B20的响应
	gpiod_direction_input(ds18b20->ds18b20_gpio);

	// 6. 等待DS18B20拉低数据线（存在脉冲开始）
	while(gpiod_get_value(ds18b20->ds18b20_gpio));

	// 7. 等待DS18B20释放数据线（存在脉冲结束）
	while(!gpiod_get_value(ds18b20->ds18b20_gpio));

	// 8. 等待480us，确保复位时序完成
	udelay(480);
}

/**
 * ds18b20_writebit - DS18B20写位函数
 * @bit: 要写入的位值（0或1）
 * 功能：实现DS18B20的写位操作，这是1-Wire通信协议的核心功能
 * 
 * DS18B20写位时序：
 * 1. 主机拉低数据线开始写时隙
 * 2. 根据要写入的位值决定释放时间：
 *    - 写1：拉低1-15us后释放
 *    - 写0：拉低60-120us后释放
 * 3. 等待写时隙结束
 */
void ds18b20_writebit(unsigned char bit){
	
	// 1. 设置GPIO为输出模式，输出高电平
	gpiod_direction_output(ds18b20->ds18b20_gpio,1);

	// 2. 拉低数据线，开始写时隙
	gpiod_set_value(ds18b20->ds18b20_gpio,0);

	// 3. 根据位值决定释放时间
	if(bit)
	{
		// 写1：拉低10us后释放，DS18B20在15-60us内采样
		udelay(10);
		
		// 释放数据线，输出高电平
		gpiod_direction_output(ds18b20->ds18b20_gpio,1);
	
	}

	// 4. 等待写时隙结束（总共65us）
	udelay(65);

	// 5. 确保GPIO为输出模式，输出高电平
	gpiod_direction_output(ds18b20->ds18b20_gpio,1);

	// 6. 额外等待2us，确保时序稳定
	udelay(2);
}

/**
 * ds18b20_writebyte - DS18B20写字节函数
 * @data: 要写入的字节数据
 * 功能：通过调用写位函数，逐位写入一个字节的数据
 * 
 * 写入顺序：从最低位（LSB）开始，逐位向最高位（MSB）写入
 */
void ds18b20_writebyte(int data){
	int i;
	
	// 循环8次，逐位写入
	for(i=0;i<8;i++){
		// 写入当前最低位（data & 0x01）
		ds18b20_writebit(data & 0x01);
		
		// 数据右移一位，准备写入下一位
		data=data>>1;
	}
}

/**
 * ds18b20_readbit - DS18B20读位函数
 * 功能：实现DS18B20的读位操作，这是1-Wire通信协议读取数据的基础
 * 返回值：读取到的位值（0或1）
 * 
 * DS18B20读位时序：
 * 1. 主机拉低数据线至少1us，开始读时隙
 * 2. 主机释放数据线，切换为输入模式
 * 3. DS18B20在15us内输出数据位
 * 4. 主机在15us后采样数据线状态
 * 5. 等待读时隙结束
 */
unsigned char ds18b20_readbit(void){

	unsigned char bit;
	
	// 1. 设置GPIO为输出模式，输出高电平
	gpiod_direction_output(ds18b20->ds18b20_gpio,1);
	
	// 2. 拉低数据线，开始读时隙（至少1us）
	gpiod_set_value(ds18b20->ds18b20_gpio,0);
	udelay(2);  // 拉低2us，确保满足1us要求

	// 3. 切换GPIO为输入模式，准备读取DS18B20的输出
	gpiod_direction_input(ds18b20->ds18b20_gpio);
	
	// 4. 等待10us，让DS18B20输出数据位
	udelay(10);

	// 5. 采样数据线状态，获取位值
	bit=gpiod_get_value(ds18b20->ds18b20_gpio);
	
	// 6. 等待60us，确保读时隙完整
	udelay(60);

	return bit;  // 返回读取到的位值
}

/**
 * ds18b20_readbyte - DS18B20读字节函数
 * 功能：通过调用读位函数，逐位读取一个字节的数据
 * 返回值：读取到的完整字节数据
 * 
 * 读取顺序：从最低位（LSB）开始，逐位向最高位（MSB）读取
 */
int ds18b20_readbyte(void){
	
	int data=0;  // 初始化数据为0
	int i;

	// 循环8次，逐位读取
	for(i=0;i<8;i++){
		// 读取当前位，并左移到对应位置
		// 使用位或运算（|=）将读取的位组合成完整字节
		data |= ds18b20_readbit()<<i;
	}

	return data;  // 返回完整的字节数据
}

/**
 * ds18b20_readtemp - DS18B20温度读取函数
 * 功能：实现完整的温度读取流程，包括温度转换和温度数据读取
 * 返回值：原始温度数据（16位，包含符号位和温度值）
 * 
 * DS18B20温度读取流程：
 * 1. 发送复位信号
 * 2. 发送跳过ROM命令（0xCC）
 * 3. 发送开始转换命令（0x44）
 * 4. 等待转换完成（750ms）
 * 5. 发送复位信号
 * 6. 发送跳过ROM命令（0xCC）
 * 7. 发送读取暂存器命令（0xBE）
 * 8. 读取温度低字节和高字节
 * 9. 组合成完整的16位温度数据
 */
int ds18b20_readtemp(void){

	int temp_l,temp_h,temp;  // 温度低字节、高字节和组合后的温度值

	// 1. 发送复位信号，准备开始温度转换
	ds18b20_reset();
	
	// 2. 发送跳过ROM命令（0xCC），跳过ROM地址匹配
	ds18b20_writebyte(0xCC);
	
	// 3. 发送开始转换命令（0x44），启动温度转换
	ds18b20_writebyte(0x44);
	
	// 4. 等待温度转换完成，DS18B20需要750ms完成转换
	mdelay(750);

	// 5. 发送复位信号，准备读取温度数据
	ds18b20_reset();
	
	// 6. 发送跳过ROM命令（0xCC），跳过ROM地址匹配
	ds18b20_writebyte(0xCC);
	
	// 7. 发送读取暂存器命令（0xBE），准备读取温度数据
	ds18b20_writebyte(0xBE);

	// 8. 读取温度低字节（LSB）
	temp_l=ds18b20_readbyte();
	
	// 9. 读取温度高字节（MSB）
	temp_h=ds18b20_readbyte();

	// 10. 将高字节左移8位，与低字节组合成16位温度数据
	temp_h=temp_h << 8;
	temp=temp_h|temp_l;

	return temp;  // 返回16位温度数据
}

/**
 * ds18b20_open - 字符设备打开函数
 * @inode: 索引节点指针
 * @file: 文件结构体指针
 * 返回值：0表示成功
 */
int ds18b20_open(struct inode *inode,struct file *file)
{
	return 0;  // 打开成功，返回0
}

/**
 * ds18b20_read - 字符设备读取函数
 * @file: 文件结构体指针
 * @buf: 用户空间缓冲区指针
 * @size: 要读取的字节数
 * @offs: 文件偏移量指针
 * 返回值：0表示成功，-1表示失败
 * 
 * 功能：实现用户空间读取温度数据的接口
 * 当用户程序调用read()函数时，会触发此函数读取温度数据
 */
ssize_t ds18b20_read(struct file *fle,char __user *buf, size_t size,loff_t *offs)
{

	int ds18b20_temp;  // 存储读取到的温度数据
	
	// 调用温度读取函数，获取DS18B20的温度数据
	ds18b20_temp=ds18b20_readtemp();

	// 将温度数据从内核空间复制到用户空间
	// copy_to_user函数确保数据安全传输
	if(copy_to_user(buf,&ds18b20_temp,sizeof(ds18b20_temp))){
		return -1;  // 复制失败，返回错误
	}

	return 0;  // 复制成功，返回0
}

/**
 * ds18b20_release - 字符设备释放函数
 * @inode: 索引节点指针
 * @file: 文件结构体指针
 * 返回值：0表示成功
 */
int ds18b20_release(struct inode *inode,struct file *file)
{
	return 0;  // 释放成功，返回0
}

/**
 * ds18b20_fops - 文件操作结构体
 * 定义了字符设备支持的文件操作函数
 */
struct file_operations ds18b20_fops={
	.open=ds18b20_open,        // 打开设备
	.read=ds18b20_read,        // 读取设备（温度数据）
	.release=ds18b20_release,  // 释放设备
	.owner=THIS_MODULE,        // 模块所有者
};

/**
 * ds18b20_probe - 平台驱动探测函数
 * @dev: 平台设备结构体指针
 * 功能：创建设备节点、字符设备、设备类、GPIO控制等
 * 返回值：0表示成功，负值表示失败
 */
int ds18b20_probe(struct platform_device *dev){
	int ret;
	printk("This is probe \n");  // 打印探测信息

	// 分配设备数据结构体内存
	ds18b20=kzalloc(sizeof(*ds18b20),GFP_KERNEL);
	if(ds18b20==NULL)
	{
		printk("kzalloc error\n");  // 内存分配失败
		ret=-ENOMEM;
		goto error_0;  // 跳转到错误处理
	}
	
	// 分配字符设备号
	ret=alloc_chrdev_region(&ds18b20->dev_num,0,1,"myds18b20");
	if(ret<0)
	{
		printk("alloc_chrdev_region error\n");  // 设备号分配失败
		ret=-EAGAIN;
		goto error_1;  // 跳转到错误处理
	}
	
	// 初始化字符设备
	cdev_init(&ds18b20->ds18b20_cdev,&ds18b20_fops);
	ds18b20->ds18b20_cdev.owner=THIS_MODULE;
	cdev_add(&ds18b20->ds18b20_cdev,ds18b20->dev_num,1);

	// 创建设备类
	ds18b20->ds18b20_class=class_create(THIS_MODULE,"sensors");
	if(IS_ERR(ds18b20->ds18b20_class))
	{
		printk("class_create error\n");  // 设备类创建失败
		ret=PTR_ERR(ds18b20->ds18b20_class);
		goto error_2;  // 跳转到错误处理
	}

	// 创建设备节点
	ds18b20->ds18b20_device=device_create(ds18b20->ds18b20_class,NULL,ds18b20->dev_num,NULL,"ds18b20");
	if(IS_ERR(ds18b20->ds18b20_device))
	{
		printk("device_create error\n");  // 设备节点创建失败
		ret=PTR_ERR(ds18b20->ds18b20_device);
		goto error_3;  // 跳转到错误处理
	}
	
	// 获取GPIO描述符，从设备树中读取名为"ds18b20"的GPIO
	ds18b20->ds18b20_gpio=gpiod_get_optional(&dev->dev,"ds18b20",0);
	if(ds18b20->ds18b20_gpio==NULL)
	{
		ret=-EBUSY;  // GPIO获取失败，返回忙状态错误
		goto error_4;  // 跳转到错误处理
	}

	// 设置GPIO为输出模式，初始值为高电平
	// 这是DS18B20通信协议的要求，数据线需要保持高电平状态
	gpiod_direction_output(ds18b20->ds18b20_gpio,1);

	return 0;  // 成功返回

	// 错误处理标签，按创建顺序反向清理资源
error_4:
	device_destroy(ds18b20->ds18b20_class,ds18b20->dev_num);  // 销毁设备节点

error_3:
	class_destroy(ds18b20->ds18b20_class);  // 销毁设备类
error_2:
	cdev_del(&ds18b20->ds18b20_cdev);       // 删除字符设备
	unregister_chrdev_region(ds18b20->dev_num,1);  // 注销设备号
error_1:
	kfree(ds18b20);                          // 释放内存
error_0:
	return ret;                               // 返回错误码
}

/**
 * ds18b20_match_table - 设备树匹配表
 * 用于在设备树中匹配compatible属性为"ds18b20"的节点
 */
const struct of_device_id ds18b20_match_table[]={
	{.compatible="ds18b20"},  // 匹配设备树中的ds18b20节点
	{},                       // 空项，表示匹配表结束
};

/**
 * ds18b20_driver - 平台驱动结构体
 * 定义了驱动的基本信息，包括名称、匹配表、探测函数等
 */
struct platform_driver ds18b20_driver={
	.driver={
		.owner=THIS_MODULE,                    // 模块所有者
		.name="ds18b20",                       // 驱动名称
		.of_match_table=ds18b20_match_table,   // 设备树匹配表
	},
	.probe=ds18b20_probe,                     // 探测函数指针
};

/**
 * ds18b20_init - 模块初始化函数
 * 在模块加载时被调用，负责注册平台驱动到内核
 * 返回值：0表示成功，负值表示失败
 */
static int __init ds18b20_init(void){
	int ret;

	// 注册平台驱动到内核
	ret=platform_driver_register(&ds18b20_driver);
	if(ret<0){
		printk("platform_driver_register error\n");  // 注册失败时打印错误信息
		return -1;
	}

	return 0;  // 注册成功
}

/**
 * ds18b20_exit - 模块退出函数
 * 在模块卸载时被调用，负责清理所有资源
 */
static void __exit ds18b20_exit(void){

	// 按创建顺序反向清理资源
	gpiod_put(ds18b20->ds18b20_gpio);	                    // 释放GPIO资源
	device_destroy(ds18b20->ds18b20_class,ds18b20->dev_num);  // 销毁设备节点
	class_destroy(ds18b20->ds18b20_class);                     // 销毁设备类
	cdev_del(&ds18b20->ds18b20_cdev);                          // 删除字符设备
	unregister_chrdev_region(ds18b20->dev_num,1);              // 注销设备号
	kfree(ds18b20);                                             // 释放内存

	// 注销平台驱动
	platform_driver_unregister(&ds18b20_driver);
}

// 模块初始化和退出宏，指定加载和卸载时调用的函数
module_init(ds18b20_init);    // 模块加载时调用ds18b20_init
module_exit(ds18b20_exit);    // 模块卸载时调用ds18b20_exit

// 模块许可证声明
MODULE_LICENSE("GPL");


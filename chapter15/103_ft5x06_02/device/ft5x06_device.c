/*
 * FT5X06触摸屏I2C设备注册模块
 * 
 * 本模块负责动态注册FT5X06触摸屏设备到I2C总线
 * 通过i2c_board_info结构体定义设备信息，然后注册到指定的I2C适配器
 * 
 */

#include <linux/module.h>    // 内核模块基本功能
#include <linux/init.h>      // 模块初始化和退出宏定义
#include <linux/i2c.h>       // I2C总线驱动接口

// 全局I2C适配器指针，用于设备注册和清理
struct i2c_adapter *i2c_ada;

/**
 * ft5x06 - I2C设备板级信息结构体数组
 * 
 * 定义要注册的FT5X06设备的基本信息
 * 包括设备类型和I2C地址
 */
static struct i2c_board_info ft5x06[]={
	{
		.type="my-ft5x06",  // 设备类型，必须与驱动中的id_table匹配
		.addr=0x38,         // I2C设备地址
	},
};

/**
 * ft5x06_client_init - 设备注册初始化函数
 * 
 * 模块加载时调用，负责：
 * 1. 获取指定的I2C适配器
 * 2. 将FT5X06设备注册到I2C总线
 * 
 * 返回值: 成功返回0，失败返回负错误码
 */
static int ft5x06_client_init(void)
{
	// 获取I2C适配器1（通常对应I2C1总线）
	i2c_ada=i2c_get_adapter(1);
	if(!i2c_ada)
	{
		printk(KERN_ERR"Failed to get I2C adapter\n");
		return -ENODEV;
	}

	// 将FT5X06设备注册到I2C总线
	// 注册后内核会自动匹配相应的驱动
	i2c_new_device(i2c_ada,ft5x06);

	return 0;
}

/**
 * ft5x06_client_exit - 设备注销清理函数
 * 
 * 模块卸载时调用，负责：
 * 1. 释放I2C适配器引用
 * 2. 清理相关资源
 */
static void ft5x06_client_exit(void)
{
	// 释放I2C适配器引用
	i2c_put_adapter(i2c_ada);
}

// 指定模块的初始化和退出函数
module_init(ft5x06_client_init);
module_exit(ft5x06_client_exit);

// 指定模块许可证
MODULE_LICENSE("GPL");

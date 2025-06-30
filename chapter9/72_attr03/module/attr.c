/*
 * attr.c - Linux内核模块示例：演示sysfs_create_file的使用
 * 
 * 本模块展示了如何使用sysfs_create_file函数手动创建sysfs属性文件，
 * 相比之前的模块，这里不使用kobj_type的default_attrs，而是通过
 * sysfs_create_file函数动态添加属性文件到已存在的kobject。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口
#include<linux/sysfs.h>     // 提供sysfs相关接口

// 声明kobject指针，用于演示sysfs_create_file的使用
struct kobject *mykobject01;

/**
 * show_myvalue1 - value1属性的读取函数
 * 
 * 功能：处理用户空间对value1属性的读取操作（如cat命令）
 * 
 * 参数：
 * @kobj: 指向kobject的指针
 * @attr: 指向kobj_attribute的指针
 * @buf: 输出缓冲区，用于存储要返回给用户空间的数据
 * 
 * 返回值：写入缓冲区的字节数
 */
ssize_t show_myvalue1(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	ssize_t count;
	// 将固定字符串写入缓冲区
	count = sprintf(buf,"show_myvalue1\n");
	return count;
}

/**
 * store_myvalue1 - value1属性的写入函数
 * 
 * 功能：处理用户空间对value1属性的写入操作（如echo命令）
 * 
 * 参数：
 * @kobj: 指向kobject的指针
 * @attr: 指向kobj_attribute的指针
 * @buf: 输入缓冲区，包含用户空间写入的数据
 * @count: 输入数据的字节数
 * 
 * 返回值：处理的字节数
 */
ssize_t store_myvalue1(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
	// 打印用户空间写入的数据到内核日志
	printk("buf is %s\n",buf);
	return count;
}

/**
 * show_myvalue2 - value2属性的读取函数
 * 
 * 功能：处理用户空间对value2属性的读取操作
 */
ssize_t show_myvalue2(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	ssize_t count;
	// 将固定字符串写入缓冲区
	count = sprintf(buf,"show_myvalue2\n");
	return count;
}

/**
 * store_myvalue2 - value2属性的写入函数
 * 
 * 功能：处理用户空间对value2属性的写入操作
 */
ssize_t store_myvalue2(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
	// 打印用户空间写入的数据到内核日志
	printk("buf is %s\n",buf);
	return count;
}

// 使用__ATTR宏定义kobj_attribute结构体
// 参数1："value1" - 属性名称
// 参数2：0664 - 文件权限（所有者读写，组读写，其他用户只读）
// 参数3：show_myvalue1 - 读取函数指针
// 参数4：store_myvalue1 - 写入函数指针
struct kobj_attribute value1=__ATTR(value1,0664,show_myvalue1,store_myvalue1);
struct kobj_attribute value2=__ATTR(value2,0664,show_myvalue2,store_myvalue2);

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 使用kobject_create_and_add创建kobject并添加到sysfs
 * 2. 使用sysfs_create_file手动创建属性文件
 * 3. 将属性文件添加到kobject中
 * 
 * 返回值：最后一个sysfs_create_file的返回值
 */
static int mykobj_init(void)
{
	int ret;

	// 创建kobject并添加到sysfs根目录
	// 注意：这里使用kobject_create_and_add，不需要自定义kobj_type
	mykobject01=kobject_create_and_add("mykobject01",NULL);

	// 使用sysfs_create_file手动创建属性文件
	// 参数1：mykobject01 - 要添加属性的kobject
	// 参数2：&value1.attr - 指向attribute结构体的指针
	ret=sysfs_create_file(mykobject01,&value1.attr);
	
	// 创建第二个属性文件
	ret=sysfs_create_file(mykobject01,&value2.attr);

	return ret;
}

/**
 * mykobj_exit - 模块退出函数
 * 
 * 功能：
 * 调用kobject_put()减少kobject的引用计数
 * 当引用计数为0时，kobject会自动释放，同时也会释放其下的所有属性文件
 * 
 * 注意：这里不需要手动删除属性文件，kobject释放时会自动清理
 */
static void mykobj_exit(void)
{
	// 减少kobject的引用计数
	// 当引用计数为0时，kobject会自动释放
	kobject_put(mykobject01);
}

// 模块初始化函数注册
module_init(mykobj_init);    // 注册初始化函数，模块加载时调用
module_exit(mykobj_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

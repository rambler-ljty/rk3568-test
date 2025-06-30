/*
 * attr.c - Linux内核模块示例：演示sysfs属性的创建和操作
 * 
 * 本模块展示了如何创建自定义的kobject结构体，并为其添加sysfs属性。
 * 通过实现show和store函数，实现用户空间对内核数据的读写操作。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口
#include<linux/sysfs.h>     // 提供sysfs相关接口

/**
 * mykobj - 自定义的kobject结构体
 * 
 * 功能：
 * 扩展kobject结构体，添加自定义的数据成员
 * 通过container_of宏可以从kobject指针获取mykobj指针
 * 
 * 成员：
 * kobj: 内嵌的kobject结构体
 * value1: 第一个自定义数据成员
 * value2: 第二个自定义数据成员
 */
struct mykobj
{
	struct kobject kobj;  // 内嵌的kobject结构体
	int value1;           // 第一个数据成员
	int value2;           // 第二个数据成员
};

// 声明自定义kobject指针
struct mykobj *mykobject01;

/**
 * dynamic_kobj_release - 自定义的kobject释放函数
 * 
 * 功能：
 * 当kobject的引用计数为0时，内核会自动调用此函数
 * 使用container_of宏从kobject指针获取mykobj指针
 * 
 * 参数：
 * @kobj: 指向要释放的kobject的指针
 */
static void dynamic_kobj_release(struct kobject *kobj)
{
	// 使用container_of宏从kobject指针获取mykobj指针
	// container_of(ptr, type, member)宏的作用：
	// 已知结构体成员的指针ptr，获取结构体的指针
	struct mykobj *mykobject01=container_of(kobj,struct mykobj,kobj);
	printk("kobject:(%p):%s",kobj,__func__);
	// 释放整个mykobj结构体的内存
	kfree(mykobject01);
}

/**
 * value1 - 第一个sysfs属性
 * 
 * 功能：
 * 定义名为"value1"的sysfs属性
 * 权限设置为0666（所有用户可读写）
 */
struct attribute value1={
	.name="value1",    // 属性名称，在sysfs中显示的文件名
	.mode=0666,        // 文件权限：所有用户可读写
};

/**
 * value2 - 第二个sysfs属性
 * 
 * 功能：
 * 定义名为"value2"的sysfs属性
 * 权限设置为0666（所有用户可读写）
 */
struct attribute value2={
	.name="value2",    // 属性名称，在sysfs中显示的文件名
	.mode=0666,        // 文件权限：所有用户可读写
};

/**
 * myattr - 属性数组
 * 
 * 功能：
 * 定义属性数组，包含所有要创建的sysfs属性
 * 数组必须以NULL结尾
 */
struct attribute *myattr[]={
	&value1,           // 指向第一个属性
	&value2,           // 指向第二个属性
	NULL,              // 数组结束标记
};

/**
 * myshow - 属性读取函数
 * 
 * 功能：
 * 当用户空间读取sysfs属性时，内核会调用此函数
 * 根据属性名称返回对应的数据值
 * 
 * 参数：
 * @kobj: 指向kobject的指针
 * @attr: 指向要读取的属性的指针
 * @buf: 输出缓冲区，用于存储要返回的数据
 * 
 * 返回值：写入缓冲区的字节数
 */
ssize_t myshow(struct kobject *kobj,struct attribute *attr,char *buf)
{
	ssize_t count;
	// 使用container_of宏从kobject指针获取mykobj指针
	struct mykobj *mykobject01=container_of(kobj,struct mykobj,kobj);
	
	// 根据属性名称返回对应的数据
	if(strcmp(attr->name,"value1")==0)
	{
		// 将value1的值转换为字符串并写入缓冲区
		count=sprintf(buf,"%d\n",mykobject01->value1);
	}
	else if(strcmp(attr->name,"value2")==0)
	{
		// 将value2的值转换为字符串并写入缓冲区
		count=sprintf(buf,"%d\n",mykobject01->value2);
	}
	else
	{
		// 未知属性，返回0
		count=0;
	}
	return count;
}

/**
 * mystore - 属性写入函数
 * 
 * 功能：
 * 当用户空间写入sysfs属性时，内核会调用此函数
 * 根据属性名称更新对应的数据值
 * 
 * 参数：
 * @kobj: 指向kobject的指针
 * @attr: 指向要写入的属性的指针
 * @buf: 输入缓冲区，包含用户写入的数据
 * @size: 输入数据的字节数
 * 
 * 返回值：处理的字节数
 */
ssize_t mystore(struct kobject *kobj,struct attribute *attr,const char *buf,size_t size)
{
	// 使用container_of宏从kobject指针获取mykobj指针
	struct mykobj *mykobject01=container_of(kobj,struct mykobj,kobj);
	
	// 根据属性名称更新对应的数据
	if(strcmp(attr->name,"value1")==0)
	{
		// 从缓冲区读取数据并更新value1
		sscanf(buf,"%d\n",&mykobject01->value1);
	}
	else if(strcmp(attr->name,"value2")==0)
	{
		// 从缓冲区读取数据并更新value2
		sscanf(buf,"%d\n",&mykobject01->value2);
	}
	return size;
}

/**
 * myops - sysfs操作函数集
 * 
 * 功能：
 * 定义sysfs属性的读写操作函数
 * 当用户空间访问sysfs属性时，内核会调用这些函数
 * 
 * 成员：
 * .show: 读取属性时调用的函数
 * .store: 写入属性时调用的函数
 */
struct sysfs_ops myops = {
	.show = myshow,    // 设置读取函数
	.store = mystore,  // 设置写入函数
};

/**
 * mytype - 自定义的kobj_type结构体
 * 
 * 功能：
 * 定义kobject的类型，包含release函数、默认属性和sysfs操作函数
 * 
 * 成员：
 * .release: 释放函数指针
 * .default_attrs: 默认属性数组
 * .sysfs_ops: sysfs操作函数集
 */
static struct kobj_type mytype={
	.release=dynamic_kobj_release,  // 设置释放函数
	.default_attrs=myattr,          // 设置默认属性数组
	.sysfs_ops=&myops,              // 设置sysfs操作函数集
};

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 分配自定义kobject内存
 * 2. 初始化数据成员
 * 3. 使用自定义kobj_type初始化kobject
 * 4. 将kobject添加到sysfs文件系统
 * 
 * 返回值：0表示成功
 */
static int mykobj_init(void)
{
	int ret;

	// 分配自定义kobject内存
	mykobject01=kzalloc(sizeof(struct mykobj),GFP_KERNEL);
	// 初始化数据成员
	mykobject01->value1=1;
	mykobject01->value2=1;

	// 使用自定义kobj_type初始化kobject并添加到sysfs
	// 注意：这里传递的是&mykobject01->kobj，即内嵌的kobject指针
	ret=kobject_init_and_add(&mykobject01->kobj,&mytype,NULL,"%s","mykobject01");

	return 0;
}

/**
 * mykobj_exit - 模块退出函数
 * 
 * 功能：
 * 调用kobject_put()减少kobject的引用计数
 * 当引用计数为0时，会自动调用自定义的release函数
 */
static void mykobj_exit(void)
{
	// 减少kobject的引用计数
	// 注意：这里传递的是&mykobject01->kobj
	kobject_put(&mykobject01->kobj);
}

// 模块初始化函数注册
module_init(mykobj_init);    // 注册初始化函数，模块加载时调用
module_exit(mykobj_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

/*
 * ktype.c - Linux内核模块示例：演示kobj_type和自定义release函数
 * 
 * 本模块展示了如何定义自定义的kobj_type结构体，并实现自定义的release函数
 * 来控制kobject的释放过程。当kobject的引用计数为0时，会调用自定义的release函数。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口

// 声明kobject指针，用于演示自定义release函数
struct kobject *mykobject03;

/**
 * dynamic_kobj_release - 自定义的kobject释放函数
 * 
 * 功能：
 * 当kobject的引用计数为0时，内核会自动调用此函数
 * 此函数负责清理kobject相关的资源并释放内存
 * 
 * 参数：
 * @kobj: 指向要释放的kobject的指针
 * 
 * 注意：此函数是kobject生命周期管理的最后一步
 */
static void dynamic_kobj_release(struct kobject *kobj)
{
	// 打印调试信息，显示kobject的地址和函数名
	// %p用于打印指针地址，__func__是当前函数名的宏
	printk("kobject:(%p):%s\n",kobj,__func__);
	// 释放kobject占用的内存
	// 注意：这里释放的是kobject本身的内存，不是kobject指向的数据
	kfree(kobj);
}

/**
 * mytype - 自定义的kobj_type结构体
 * 
 * 功能：
 * 定义kobject的类型，包含release函数指针
 * 当kobject的引用计数为0时，会调用.release函数
 * 
 * 成员：
 * .release: 指向自定义释放函数的指针
 */
struct kobj_type mytype={
	.release=dynamic_kobj_release,  // 设置自定义的释放函数
};

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 使用kzalloc分配kobject内存
 * 2. 使用自定义的kobj_type初始化kobject
 * 3. 将kobject添加到sysfs文件系统
 * 
 * 返回值：0表示成功
 */
static int mykobj_init(void)
{
	int ret;

	// 使用kzalloc分配清零的kobject内存空间
	mykobject03=kzalloc(sizeof(struct kobject),GFP_KERNEL);

	// 使用自定义的kobj_type初始化kobject并添加到sysfs
	// 参数1：mykobject03 - 要初始化的kobject
	// 参数2：&mytype - 自定义的kobj_type结构体
	// 参数3：NULL - 父kobject为NULL，表示添加到sysfs根目录
	// 参数4："%s" - 格式化字符串
	// 参数5："mykobject03" - kobject的名称
	ret=kobject_init_and_add(mykobject03,&mytype,NULL,"%s","mykobject03");

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
	// 当引用计数为0时，会调用mytype.release函数（即dynamic_kobj_release）
	kobject_put(mykobject03);
}

// 模块初始化函数注册
module_init(mykobj_init);    // 注册初始化函数，模块加载时调用
module_exit(mykobj_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

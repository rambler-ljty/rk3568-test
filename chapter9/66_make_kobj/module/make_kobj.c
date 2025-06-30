/*
 * make_kobj.c - Linux内核模块示例：演示kobject的创建和管理
 * 
 * 本模块展示了三种不同的kobject创建方式：
 * 1. 使用kobject_create_and_add()创建并添加到sysfs
 * 2. 创建父子关系的kobject
 * 3. 使用kzalloc分配内存后手动初始化kobject
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口

// 声明三个kobject指针，用于演示不同的创建方式
struct kobject *mykobject01;  // 第一个kobject：使用kobject_create_and_add创建
struct kobject *mykobject02;  // 第二个kobject：作为mykobject01的子对象
struct kobject *mykobject03;  // 第三个kobject：使用kzalloc分配后手动初始化

// 定义kobject类型结构体，用于mykobject03
struct kobj_type mytype;

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 创建第一个kobject并添加到sysfs根目录
 * 2. 创建第二个kobject作为第一个的子对象
 * 3. 使用kzalloc分配内存，然后手动初始化第三个kobject
 * 
 * 返回值：0表示成功
 */
static int mykobj_init(void)
{
	int ret;
	
	// 方式1：使用kobject_create_and_add创建kobject并添加到sysfs
	// 参数1："mykobject01" - kobject的名称
	// 参数2：NULL - 父kobject为NULL，表示添加到sysfs根目录
	mykobject01=kobject_create_and_add("mykobject01",NULL);
	
	// 方式2：创建具有父子关系的kobject
	// mykobject02将作为mykobject01的子对象出现在sysfs中
	// 路径将是：/sys/mykobject01/mykobject02
	mykobject02=kobject_create_and_add("mykobject02",mykobject01);

	// 方式3：使用kzalloc分配内存，然后手动初始化
	// 分配一个kobject结构体的内存空间
	mykobject03=kzalloc(sizeof(struct kobject),GFP_KERNEL);
	
	// 手动初始化kobject并添加到sysfs
	// kobject_init_and_add会调用kobject_init初始化，然后调用kobject_add添加到sysfs
	ret=kobject_init_and_add(mykobject03,&mytype,NULL,"%s","mykobject03");

	return 0;
}

/**
 * mykobj_exit - 模块退出函数
 * 
 * 功能：
 * 清理所有创建的kobject，释放相关资源
 * kobject_put会减少引用计数，当计数为0时自动释放内存
 */
static void mykobj_exit(void)
{
	// 释放所有kobject，减少引用计数
	kobject_put(mykobject01);  // 释放第一个kobject
	kobject_put(mykobject02);  // 释放第二个kobject
	kobject_put(mykobject03);  // 释放第三个kobject
}

// 模块初始化函数注册
module_init(mykobj_init);    // 注册初始化函数，模块加载时调用
module_exit(mykobj_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

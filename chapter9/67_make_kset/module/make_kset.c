/*
 * make_kset.c - Linux内核模块示例：演示kset的创建和管理
 * 
 * 本模块展示了kset的创建方式，以及如何将多个kobject添加到同一个kset中。
 * kset是kobject的集合，用于管理具有相同类型的kobject对象。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject和kset相关接口

// 声明kobject指针，用于演示kset管理
struct kobject *mykobject01;  // 第一个kobject，将添加到mykset中
struct kobject *mykobject02;  // 第二个kobject，也将添加到mykset中
struct kset *mykset;          // kset对象，用于管理多个kobject

// 定义kobject类型结构体，用于初始化kobject
struct kobj_type mytype;

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 创建一个kset对象
 * 2. 创建两个kobject并手动分配内存
 * 3. 将两个kobject添加到同一个kset中
 * 4. 在sysfs中创建对应的目录结构
 * 
 * 返回值：0表示成功
 */
static int mykobj_init(void)
{
	int ret;
	
	// 创建kset对象
	// kset_create_and_add函数会：
	// 1. 分配kset内存
	// 2. 初始化kset
	// 3. 将kset添加到sysfs中
	// 参数1："mykset" - kset的名称
	// 参数2：NULL - 父kobject为NULL，表示添加到sysfs根目录
	// 参数3：NULL - kset_uevent_ops为NULL，使用默认的事件处理
	mykset=kset_create_and_add("mykset",NULL,NULL);

	// 创建第一个kobject
	// 使用kzalloc分配清零的内存空间
	mykobject01=kzalloc(sizeof(struct kobject),GFP_KERNEL);
	// 设置kobject所属的kset
	mykobject01->kset=mykset;
	// 初始化kobject并添加到sysfs
	// 由于设置了kset，kobject会自动添加到kset的目录下
	ret=kobject_init_and_add(mykobject01,&mytype,NULL,"%s","mykobject01");

	// 创建第二个kobject
	// 同样使用kzalloc分配内存
	mykobject02=kzalloc(sizeof(struct kobject),GFP_KERNEL);
	// 设置kobject所属的kset（与第一个kobject相同）
	mykobject02->kset=mykset;
	// 初始化kobject并添加到sysfs
	ret=kobject_init_and_add(mykobject02,&mytype,NULL,"%s","mykobject02");

	return 0;
}

/**
 * mykobj_exit - 模块退出函数
 * 
 * 功能：
 * 清理所有创建的kobject，释放相关资源
 * 注意：kset会在最后一个kobject被释放时自动释放
 */
static void mykobj_exit(void)
{
	// 释放kobject，减少引用计数
	kobject_put(mykobject01);  // 释放第一个kobject
	kobject_put(mykobject02);  // 释放第二个kobject
	// kset会在所有kobject被释放后自动释放
}

// 模块初始化函数注册
module_init(mykobj_init);    // 注册初始化函数，模块加载时调用
module_exit(mykobj_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

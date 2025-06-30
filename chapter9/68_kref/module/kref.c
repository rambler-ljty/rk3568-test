/*
 * kref.c - Linux内核模块示例：演示kref引用计数机制
 * 
 * 本模块展示了kobject中kref引用计数的工作原理，通过打印引用计数的变化
 * 来观察父子关系对引用计数的影响，以及kobject_put()对引用计数的影响。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口

// 声明三个kobject指针，用于演示引用计数变化
struct kobject *mykobject01;  // 第一个kobject，作为父对象
struct kobject *mykobject02;  // 第二个kobject，作为mykobject01的子对象
struct kobject *mykobject03;  // 第三个kobject，独立对象

// 定义kobject类型结构体，用于初始化kobject
struct kobj_type mytype;

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 创建三个kobject对象
 * 2. 建立父子关系（mykobject02作为mykobject01的子对象）
 * 3. 打印每个kobject的引用计数，观察引用计数的变化
 * 
 * 返回值：0表示成功
 */
static int mykobj_init(void)
{
	int ret;
	
	// 创建第一个kobject并添加到sysfs根目录
	mykobject01=kobject_create_and_add("mykobject01",NULL);
	// 打印mykobject01的引用计数
	// kref.refcount.refs.counter是引用计数的实际值
	printk("mykobject01 kref is %d\n",mykobject01->kref.refcount.refs.counter);

	// 创建第二个kobject作为mykobject01的子对象
	mykobject02=kobject_create_and_add("mykobject02",mykobject01);
	// 打印两个kobject的引用计数
	// 注意：当mykobject02作为mykobject01的子对象时，mykobject01的引用计数会增加
	printk("mykobject01 kref is %d\n",mykobject01->kref.refcount.refs.counter);
	printk("mykobject02 kref is %d\n",mykobject02->kref.refcount.refs.counter);

	// 创建第三个kobject，使用手动分配内存的方式
	mykobject03=kzalloc(sizeof(struct kobject),GFP_KERNEL);
	// 初始化第三个kobject并添加到sysfs
	ret=kobject_init_and_add(mykobject03,&mytype,NULL,"%s","mykobject03");
	// 打印mykobject03的引用计数
	printk("mykobject03 kref is %d\n",mykobject03->kref.refcount.refs.counter);

	return 0;
}

/**
 * mykobj_exit - 模块退出函数
 * 
 * 功能：
 * 1. 在每次调用kobject_put()前后打印引用计数
 * 2. 观察引用计数的递减过程
 * 3. 演示父子关系对引用计数的影响
 * 
 * 注意：当引用计数为0时，kobject会自动释放
 */
static void mykobj_exit(void)
{
	// 打印释放前的引用计数
	printk("mykobject01 kref is %d\n",mykobject01->kref.refcount.refs.counter);
	printk("mykobject02 kref is %d\n",mykobject02->kref.refcount.refs.counter);
	printk("mykobject03 kref is %d\n",mykobject03->kref.refcount.refs.counter);
	
	// 释放mykobject01（父对象）
	// 由于mykobject02是mykobject01的子对象，释放父对象会影响子对象
	kobject_put(mykobject01);

	// 打印释放mykobject01后的引用计数
	// 注意：mykobject02的引用计数可能会发生变化
	printk("mykobject01 kref is %d\n",mykobject01->kref.refcount.refs.counter);
	printk("mykobject02 kref is %d\n",mykobject02->kref.refcount.refs.counter);
	printk("mykobject03 kref is %d\n",mykobject03->kref.refcount.refs.counter);

	// 释放mykobject02（子对象）
	kobject_put(mykobject02);
	// 打印释放mykobject02后的引用计数
	printk("mykobject01 kref is %d\n",mykobject01->kref.refcount.refs.counter);
	printk("mykobject02 kref is %d\n",mykobject02->kref.refcount.refs.counter);
	printk("mykobject03 kref is %d\n",mykobject03->kref.refcount.refs.counter);

	// 释放mykobject03（独立对象）
	kobject_put(mykobject03);
	// 打印释放mykobject03后的引用计数
	// 此时所有kobject的引用计数应该都为0或已释放
	printk("mykobject01 kref is %d\n",mykobject01->kref.refcount.refs.counter);
	printk("mykobject02 kref is %d\n",mykobject02->kref.refcount.refs.counter);
	printk("mykobject03 kref is %d\n",mykobject03->kref.refcount.refs.counter);
}

// 模块初始化函数注册
module_init(mykobj_init);    // 注册初始化函数，模块加载时调用
module_exit(mykobj_exit);    // 注册清理函数，模块卸载时调用

MODULE_LICENSE("GPL");           // 模块许可证声明
MODULE_AUTHOR("topeet");         // 模块作者信息

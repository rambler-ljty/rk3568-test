/*
 * attr.c - Linux内核模块示例：演示kobj_attribute和__ATTR宏的使用
 * 
 * 本模块展示了如何使用kobj_attribute结构体和__ATTR宏来创建sysfs属性，
 * 相比70_attr模块，这里使用了更高级的kobj_attribute方式，提供了
 * 更清晰的属性定义和操作函数绑定。
 */

#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>    // 提供内核通用功能
#include<linux/kobject.h>   // 提供kobject相关接口
#include<linux/sysfs.h>     // 提供sysfs相关接口

// 自定义kobject结构体，包含内嵌的kobject和自定义数据
struct mykobj
{
	struct kobject kobj;    // 内嵌的kobject结构体
	int value1;             // 自定义数据成员1
	int value2;             // 自定义数据成员2
};

// 声明自定义kobject指针
struct mykobj *mykobject01;

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
// __ATTR宏会自动填充kobj_attribute结构体的所有成员
// 参数1："value1" - 属性名称
// 参数2：0664 - 文件权限（所有者读写，组读写，其他用户只读）
// 参数3：show_myvalue1 - 读取函数指针
// 参数4：store_myvalue1 - 写入函数指针
struct kobj_attribute value1=__ATTR(value1,0664,show_myvalue1,store_myvalue1);
struct kobj_attribute value2=__ATTR(value2,0664,show_myvalue2,store_myvalue2);

/**
 * dynamic_kobj_release - 自定义的kobject释放函数
 * 
 * 功能：当kobject的引用计数为0时，内核会自动调用此函数
 * 
 * 参数：
 * @kobj: 指向要释放的kobject的指针
 */
static void dynamic_kobj_release(struct kobject *kobj)
{
	// 使用container_of宏从kobject指针获取自定义结构体指针
	struct mykobj *mykobject01=container_of(kobj,struct mykobj,kobj);
	// 打印调试信息
	printk("kobject:(%p):%s",kobj,__func__);
	// 释放自定义结构体的内存
	kfree(mykobject01);
}

// 定义属性数组，包含所有要注册的属性
// 注意：这里使用的是kobj_attribute的attr成员
struct attribute *myattr[]={
	&value1.attr,  // value1属性的attr成员
	&value2.attr,  // value2属性的attr成员
	NULL,          // 数组结束标记
};

/**
 * myshow - 通用的属性读取函数
 * 
 * 功能：根据属性类型调用对应的show函数
 * 
 * 参数：
 * @kobj: 指向kobject的指针
 * @attr: 指向attribute的指针
 * @buf: 输出缓冲区
 * 
 * 返回值：写入缓冲区的字节数
 */
ssize_t myshow(struct kobject *kobj,struct attribute *attr,char *buf)
{
	ssize_t count;
	// 使用container_of宏从attribute指针获取kobj_attribute指针
	struct kobj_attribute *kobj_attr=container_of(attr,struct kobj_attribute,attr);
	// 调用kobj_attribute中定义的show函数
	count=kobj_attr->show(kobj,kobj_attr,buf);
	return count;
}

/**
 * mystore - 通用的属性写入函数
 * 
 * 功能：根据属性类型调用对应的store函数
 * 
 * 参数：
 * @kobj: 指向kobject的指针
 * @attr: 指向attribute的指针
 * @buf: 输入缓冲区
 * @size: 输入数据的字节数
 * 
 * 返回值：处理的字节数
 */
ssize_t mystore(struct kobject *kobj,struct attribute *attr,const char *buf,size_t size)
{
	// 使用container_of宏从attribute指针获取kobj_attribute指针
	struct kobj_attribute *kobj_attr=container_of(attr,struct kobj_attribute,attr);
	// 调用kobj_attribute中定义的store函数
	return kobj_attr->store(kobj,kobj_attr,buf,size);
}

// 定义sysfs操作函数集
struct sysfs_ops myops = {
	.show = myshow,   // 注册通用的show函数
	.store = mystore, // 注册通用的store函数
};

// 定义kobject类型，包含释放函数、默认属性和sysfs操作函数集
static struct kobj_type mytype={
	.release=dynamic_kobj_release,  // 自定义释放函数
	.default_attrs=myattr,          // 默认属性数组
	.sysfs_ops=&myops,              // sysfs操作函数集
};

/**
 * mykobj_init - 模块初始化函数
 * 
 * 功能：
 * 1. 分配自定义kobject结构体的内存
 * 2. 初始化自定义数据成员
 * 3. 使用自定义kobj_type初始化kobject
 * 4. 将kobject添加到sysfs文件系统
 * 
 * 返回值：0表示成功
 */
static int mykobj_init(void)
{
	int ret;

	// 分配自定义kobject结构体的内存
	mykobject01=kzalloc(sizeof(struct mykobj),GFP_KERNEL);
	// 初始化自定义数据成员
	mykobject01->value1=1;
	mykobject01->value2=1;

	// 使用自定义kobj_type初始化kobject并添加到sysfs
	// 注意：这里传递的是&mykobject01->kobj，即内嵌kobject的地址
	ret=kobject_init_and_add(&mykobject01->kobj,&mytype,NULL,"%s","mykobject01");

	return 0;
}

/**
 * mykobj_exit - 模块退出函数
 * 
 * 功能：调用kobject_put()减少kobject的引用计数
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

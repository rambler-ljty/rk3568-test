// 本模块演示了如何自定义kset、kobject及其uevent操作，向用户空间发送自定义事件
// 包含必要的内核头文件
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>
#include<linux/kobject.h>

// 全局指针，分别指向两个kobject和一个kset
struct kobject *mykobject01;
struct kobject *mykobject02;
struct kset *mykset;
struct kobj_type mytype; // kobject类型结构体，通常需要实现release等回调

// name回调：返回kset的名字
const char *myname(struct kset *kset,struct kobject *kobj)
{
	return "my_kset"; // 返回kset的名称
}

// uevent回调：向用户空间传递自定义环境变量
int myevent(struct kset *kset,struct kobject *kobj,struct kobj_uevent_env *env)
{
	add_uevent_var(env,"MYDEVICE=%s","TOPEET"); // 添加自定义环境变量MYDEVICE
	return 0;
}

// filter回调：过滤kobject，只有名字为"mykobject01"的kobject返回0（不过滤），其余返回1（过滤）
int myfilter(struct kset *kset,struct kobject *kobj)
{
	if(strcmp(kobj->name,"mykobject01")==0){ // 判断kobject名称
		return 0; // 不过滤
	}else{
		return 1; // 过滤
	}
}

// 定义kset的uevent操作集，指定filter、uevent、name回调
struct kset_uevent_ops my_uevent_ops={
	.filter=myfilter,   // 过滤函数
	.uevent=myevent,    // 事件处理函数
	.name=myname,       // 命名函数
};

// 模块初始化函数
static int mykobj_init(void)
{
	int ret;
	// 创建kset并指定uevent操作
	mykset=kset_create_and_add("mykset",&my_uevent_ops,NULL);

	// 分配并初始化第一个kobject
	mykobject01=kzalloc(sizeof(struct kobject),GFP_KERNEL); // 分配内存
	mykobject01->kset=mykset; // 关联到kset
	ret=kobject_init_and_add(mykobject01,&mytype,NULL,"%s","mykobject01"); // 初始化并添加到内核

	// 分配并初始化第二个kobject
	mykobject02=kzalloc(sizeof(struct kobject),GFP_KERNEL);
	mykobject02->kset=mykset;
	ret=kobject_init_and_add(mykobject02,&mytype,NULL,"%s","mykobject02");

	// 触发第一个kobject的KOBJ_CHANGE事件，向用户空间发送uevent
	ret=kobject_uevent(mykobject01,KOBJ_CHANGE);

	// 触发第二个kobject的KOBJ_ADD事件，向用户空间发送uevent
	ret=kobject_uevent(mykobject02,KOBJ_ADD);

	return 0;
}

// 模块退出函数，释放资源
static void mykobj_exit(void)
{
	kobject_put(mykobject01); // 释放第一个kobject
	kobject_put(mykobject02); // 释放第二个kobject
	kset_unregister(mykset);  // 注销kset
}

module_init(mykobj_init);    // 注册初始化函数
module_exit(mykobj_exit);    // 注册清理函数

MODULE_LICENSE("GPL");           // 模块许可证
MODULE_AUTHOR("topeet");         // 模块作者

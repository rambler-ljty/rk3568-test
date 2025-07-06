#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>
#include<linux/kobject.h>

struct kobject *mykobject01;
struct kset *mykset;
struct kobj_type mytype;


static int mykobj_init(void)
{
	int ret;
	mykset=kset_create_and_add("mykset",NULL,NULL);

	mykobject01=kzalloc(sizeof(struct kobject),GFP_KERNEL);
	mykobject01->kset=mykset;

	ret=kobject_init_and_add(mykobject01,&mytype,NULL,"%s","mykobject01");

	ret=kobject_uevent(mykobject01,KOBJ_CHANGE);

	return 0;

}

static void mykobj_exit(void)
{
	kobject_put(mykobject01);

}


module_init(mykobj_init);    // 注册初始化函数
module_exit(mykobj_exit);    // 注册清理函数

MODULE_LICENSE("GPL");           // 模块许可证
MODULE_AUTHOR("topeet");         // 模块作者

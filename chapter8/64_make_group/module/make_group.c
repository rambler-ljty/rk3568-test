#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口

// 定义configfs组结构体
static struct config_group mygroup;

// 定义自定义项目结构体，用于存储configfs项目的数据
struct myitem
{
	struct config_item item;    // 基础configfs项目结构
	int size;                   // 数据大小
	void *addr;                 // 数据地址
};

// 定义自定义组结构体，用于创建子组
struct mygroup
{
	struct config_group group;  // 基础configfs组结构
};

// 项目释放函数，当项目被删除时调用，负责清理资源
void myitem_release(struct config_item *item)
{
	struct myitem *myitem=container_of(item,struct myitem,item);
	kfree(myitem);             // 释放项目占用的内存
	printk("%s\n",__func__);   // 打印函数名，用于调试
}

// 读取属性函数，当读取属性文件时调用
ssize_t myread_show(struct config_item *item,char *page)
{
	struct myitem *myitem=container_of(item,struct myitem,item);
	memcpy(page,myitem->addr,myitem->size);  // 复制数据到缓冲区
	printk("%s\n",__func__);                 // 打印函数名，用于调试
	return myitem->size;                     // 返回数据大小
}

// 写入属性函数，当写入属性文件时调用
ssize_t mywrite_store(struct config_item *item,const char *page,size_t size)
{
	struct myitem *myitem=container_of(item,struct myitem,item);
	myitem->addr=kmemdup(page,size,GFP_KERNEL);  // 复制数据到内核空间
	myitem->size=size;                            // 保存数据大小
	printk("%s\n",__func__);                      // 打印函数名，用于调试
	return myitem->size;                          // 返回数据大小
}

// 定义只读属性
CONFIGFS_ATTR_RO(my,read);

// 定义只写属性
CONFIGFS_ATTR_WO(my,write);

// 定义属性数组
struct configfs_attribute *my_attrs[]={
	&myattr_read,    // 只读属性
	&myattr_write,   // 只写属性
	NULL,            // 数组结束标记
};

// 定义项目操作函数集
struct configfs_item_operations myitem_ops={
	.release=myitem_release,   // 设置释放函数
};

// 定义组项目类型
static struct config_item_type mygroup_item_type={
	.ct_owner=THIS_MODULE,     // 模块所有者
	.ct_item_ops=&myitem_ops,  // 项目操作函数集
	.ct_attrs=my_attrs,        // 属性数组
};

// 定义组类型
static struct config_item_type mygroup_type={
	.ct_owner=THIS_MODULE,     // 模块所有者
};

// 创建新项目的函数，当在组目录下执行mkdir命令时调用
struct config_item *mygroup_make_item(struct config_group *group,const char *name)
{
	struct myitem *myconfig_item;
	printk("%s\n",__func__);   // 打印函数名，用于调试
	myconfig_item=kzalloc(sizeof(*myconfig_item),GFP_KERNEL);  // 分配内存
	config_item_init_type_name(&myconfig_item->item,name,&mygroup_item_type);  // 初始化项目
	return &myconfig_item->item;
}

// 删除项目的函数，当在组目录下执行rmdir命令时调用
void mygroup_delete_item(struct config_group *group, struct config_item *item)
{
	struct myitem *myitem=container_of(item,struct myitem,item);
	config_item_put(&myitem->item);  // 减少项目引用计数，可能触发释放
	printk("%s\n",__func__);         // 打印函数名，用于调试
}

// 创建新组的函数，当在组目录下执行mkdir命令时调用
struct config_group *mygroup_make_group(struct config_group *group,const char *name)
{
	struct mygroup *mygroup;
	printk("%s\n",__func__);   // 打印函数名，用于调试
	mygroup=kzalloc(sizeof(*mygroup),GFP_KERNEL);  // 分配内存
	config_group_init_type_name(&mygroup->group,name,&mygroup_type);  // 初始化组
	return &mygroup->group;
};

// 定义组操作函数集
struct configfs_group_operations mygroup_ops={
	.make_item=mygroup_make_item,    // 设置创建项目的函数
	.drop_item=mygroup_delete_item,  // 设置删除项目的函数
	.make_group=mygroup_make_group,  // 设置创建组的函数
};

// 定义组配置项目类型
static const struct config_item_type mygroup_config_item_type={
	.ct_owner=THIS_MODULE,         // 模块所有者
	.ct_group_ops=&mygroup_ops,    // 组操作函数集
};

// 定义子系统项目类型
static const struct config_item_type myconfig_item_type={
	.ct_owner=THIS_MODULE,         // 模块所有者
	.ct_group_ops=NULL,            // 组操作函数集
};

// 定义configfs子系统
static struct configfs_subsystem myconfigfs_subsystem={
	.su_group={
		.cg_item={
			.ci_namebuf="myconfigfs",    // 子系统名称
			.ci_type=&myconfig_item_type, // 项目类型
		},
	},
};

// 模块初始化函数，在模块加载时调用
static int myconfig_group_init(void)
{
	config_group_init(&myconfigfs_subsystem.su_group);  // 初始化configfs组
	configfs_register_subsystem(&myconfigfs_subsystem); // 注册configfs子系统
	config_group_init_type_name(&mygroup,"mygroup",&mygroup_config_item_type);  // 初始化mygroup组
	configfs_register_group(&myconfigfs_subsystem.su_group,&mygroup);  // 注册mygroup组
	return 0;
}

// 模块退出函数，在模块卸载时调用
static void myconfig_group_exit(void)
{
	configfs_unregister_subsystem(&myconfigfs_subsystem);  // 注销configfs子系统
}

// 注册模块的初始化和清理函数
module_init(myconfig_group_init);    // 注册初始化函数
module_exit(myconfig_group_exit);    // 注册清理函数

MODULE_LICENSE("GPL");           // 模块许可证
MODULE_AUTHOR("topeet");         // 模块作者

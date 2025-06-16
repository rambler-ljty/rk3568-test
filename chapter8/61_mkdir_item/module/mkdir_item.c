/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口

/* 定义configfs组结构体 */
static struct config_group mygroup;

/* 定义自定义项目结构体
 * 用于存储configfs项目的数据
 */
struct myitem
{
	struct config_item item;    // 基础configfs项目结构
};

/* 项目释放函数
 * 当项目被删除时调用，负责清理资源
 */
void myitem_release(struct config_item *item)
{
	struct myitem *myitem=container_of(item,struct myitem,item);
	kfree(myitem);             // 释放项目占用的内存
	printk("%s\n",__func__);   // 打印函数名，用于调试
}

/* 定义项目操作函数集 */
struct configfs_item_operations myitem_ops={
	.release=myitem_release,   // 设置释放函数
};

/* 定义组项目类型 */
static struct config_item_type mygroup_item_type={
	.ct_owner=THIS_MODULE,     // 模块所有者
	.ct_item_ops=&myitem_ops,  // 项目操作函数集
};

/* 创建新项目的函数
 * 当在组目录下执行mkdir命令时调用
 * 参数:
 *   group: 父组
 *   name: 新项目的名称
 * 返回值: 新创建的项目指针
 */
struct config_item *mygroup_make_item(struct config_group *group,const char *name)
{
	struct myitem *myconfig_item;
	printk("%s\n",__func__);   // 打印函数名，用于调试
	myconfig_item=kzalloc(sizeof(*myconfig_item),GFP_KERNEL);  // 分配内存
	config_item_init_type_name(&myconfig_item->item,name,&mygroup_item_type);  // 初始化项目
	return &myconfig_item->item;
}

/* 定义组操作函数集 */
struct configfs_group_operations mygroup_ops={
	.make_item=mygroup_make_item,  // 设置创建项目的函数
};

/* 定义组配置项目类型 */
static const struct config_item_type mygroup_config_item_type={
	.ct_owner=THIS_MODULE,         // 模块所有者
	.ct_group_ops=&mygroup_ops,    // 组操作函数集
};

/* 定义子系统项目类型 */
static const struct config_item_type myconfig_item_type={
	.ct_owner=THIS_MODULE,         // 模块所有者
	.ct_group_ops=NULL,            // 组操作函数集
};

/* 定义configfs子系统 */
static struct configfs_subsystem myconfigfs_subsystem={
	.su_group={
		.cg_item={
			.ci_namebuf="myconfigfs",    // 子系统名称
			.ci_type=&myconfig_item_type, // 项目类型
		},
	},
};

/* 模块初始化函数
 * 在模块加载时调用，完成以下工作：
 * 1. 初始化configfs组
 * 2. 注册configfs子系统
 * 3. 初始化并注册mygroup组
 * 返回值: 成功返回0，失败返回负值
 */
static int myconfig_group_init(void)
{
	// 初始化configfs组
	config_group_init(&myconfigfs_subsystem.su_group);

	// 注册configfs子系统
	configfs_register_subsystem(&myconfigfs_subsystem);

	// 初始化mygroup组，设置名称和类型
	config_group_init_type_name(&mygroup,"mygroup",&mygroup_config_item_type);

	// 将mygroup注册到子系统中
	configfs_register_group(&myconfigfs_subsystem.su_group,&mygroup);

	return 0;
}

/* 模块退出函数
 * 在模块卸载时调用，注销configfs子系统
 */
static void myconfig_group_exit(void)
{
	// 注销configfs子系统
	configfs_unregister_subsystem(&myconfigfs_subsystem);
}

/* 注册模块的初始化和清理函数 */
module_init(myconfig_group_init);    // 注册初始化函数
module_exit(myconfig_group_exit);    // 注册清理函数

MODULE_LICENSE("GPL");           // 模块许可证
MODULE_AUTHOR("topeet");         // 模块作者

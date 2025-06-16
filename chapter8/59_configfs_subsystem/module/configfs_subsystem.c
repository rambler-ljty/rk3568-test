/* 包含必要的头文件 */
#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口

/* 定义configfs项目类型
 * 用于描述configfs项目的属性和操作
 */
static const struct config_item_type myconfig_item_type={
	.ct_owner=THIS_MODULE,      // 模块所有者
	.ct_item_ops=NULL,          // 项目操作函数集
	.ct_group_ops=NULL,         // 组操作函数集
	.ct_attrs=NULL,             // 属性列表
};

/* 定义configfs子系统
 * 用于在/sys/config/下创建子系统目录
 */
static struct configfs_subsystem myconfigfs_subsystem={
	.su_group={
		.cg_item={
			.ci_namebuf="myconfigfs",    // 子系统名称，将显示在/sys/config/下
			.ci_type=&myconfig_item_type, // 指向项目类型
		},
	},
};

/* 模块初始化函数
 * 在模块加载时调用，注册configfs子系统
 * 返回值: 成功返回0，失败返回负值
 */
static int myconfigfs_init(void)
{
	// 初始化configfs组
	config_group_init(&myconfigfs_subsystem.su_group);

	// 注册configfs子系统
	configfs_register_subsystem(&myconfigfs_subsystem);

	return 0;
}

/* 模块退出函数
 * 在模块卸载时调用，注销configfs子系统
 */
static void myconfigfs_exit(void)
{
	// 注销configfs子系统
	configfs_unregister_subsystem(&myconfigfs_subsystem);
}

/* 注册模块的初始化和清理函数 */
module_init(myconfigfs_init);    // 注册初始化函数
module_exit(myconfigfs_exit);    // 注册清理函数

MODULE_LICENSE("GPL");           // 模块许可证
MODULE_AUTHOR("topeet");         // 模块作者

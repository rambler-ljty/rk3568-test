/*
 * 这是一个Linux内核模块示例，演示了字符设备驱动与内核定时器的结合使用
 * 该模块实现了一个简单的计时器设备，每秒更新一次计数值
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/atomic.h>

/* 设备结构体定义 */
struct device_test{
	dev_t dev_num;        // 设备号
	int major;            // 主设备号
	int minor;            // 次设备号
	struct cdev cdev_test; // 字符设备结构体
	struct class *class;   // 设备类
	struct device *device;  // 设备结构体
	int sec;              // 计时器计数值
};

/* 定义原子变量用于计数 */
atomic64_t v=ATOMIC_INIT(0);
/* 定义设备实例 */
struct device_test dev1;

/* 定时器回调函数声明 */
static void function_test(struct timer_list *t);

/* 定义定时器结构体，并指定回调函数 */
DEFINE_TIMER(timer_test,function_test);

/* 定时器回调函数实现 */
static void function_test(struct timer_list *t)
{
	// 原子操作增加计数值
	atomic64_inc(&v);
	// 更新设备结构体中的计数值
	dev1.sec=atomic_read(&v);
	// 重新设置定时器，1秒后再次触发
	mod_timer(&timer_test,jiffies_64+msecs_to_jiffies(1000));
}

/* 打开设备函数 */
static int cdev_test_open(struct inode *inode,struct file *file)
{
	// 设置私有数据
	file->private_data=&dev1;
	// 启动定时器
	add_timer(&timer_test);
	return 0;
}

/* 读设备函数 */
static ssize_t cdev_test_read(struct file *file,char __user *buf,size_t size,loff_t *off)
{
	// 将计数值复制到用户空间
	if(copy_to_user(buf,&dev1.sec,sizeof(dev1.sec)))
	{
		printk("copy_to_user error\n");
		return -1;
	}
	return 0;
}

/* 关闭设备函数 */
static int cdev_test_release(struct inode *inode,struct file *file)
{
	// 删除定时器
	del_timer(&timer_test);
	return 0;
}

/* 文件操作结构体 */
struct file_operations cdev_test_fops={
	.owner=THIS_MODULE,
	.open=cdev_test_open,
	.read=cdev_test_read,
	.release=cdev_test_release,
};

/* 模块初始化函数 */
static int __init timer_dev_init(void)
{
	int ret;
	
	// 分配设备号
	if(alloc_chrdev_region(&dev1.dev_num,0,1,"alloc_name")<0){
		goto err_chrdev;
	}
	printk("alloc_chrdev_region is ok\n");
	dev1.major=MAJOR(dev1.dev_num);
	dev1.minor=MINOR(dev1.dev_num);	
	printk("major is %d\n",dev1.major);
	printk("minor is %d\n",dev1.minor);

	// 初始化字符设备
	dev1.cdev_test.owner=THIS_MODULE;
	cdev_init(&dev1.cdev_test,&cdev_test_fops);

	// 添加字符设备
	ret=cdev_add(&dev1.cdev_test,dev1.dev_num,1);
	if(ret<0)
	{
		goto err_chr_add;
	}
	
	// 创建设备类
	dev1.class=class_create(THIS_MODULE,"test");
	
	if(IS_ERR(dev1.class))
	{
		ret=PTR_ERR(dev1.class);
		goto err_class_create;
	}

	// 创建设备节点
	dev1.device=device_create(dev1.class,NULL,dev1.dev_num,NULL,"test");
	if(IS_ERR(dev1.device))
	{
		ret=PTR_ERR(dev1.device);
		goto err_device_create;
	}

	return 0;

	// 错误处理
	err_device_create:
		class_destroy(dev1.class);

	err_class_create:
		cdev_del(&dev1.cdev_test);

	err_chr_add:
		unregister_chrdev_region(dev1.dev_num,1);

	err_chrdev:
		return ret;
}

/* 模块退出函数 */
static void __exit timer_dev_exit(void)
{
	// 清理设备相关资源
	device_destroy(dev1.class,dev1.dev_num);
	class_destroy(dev1.class);
	cdev_del(&dev1.cdev_test);
	unregister_chrdev_region(dev1.dev_num,1);
	printk("module exit\n");
}

// 模块入口和出口
module_init(timer_dev_init);
module_exit(timer_dev_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

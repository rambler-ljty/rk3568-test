/*
 * 这是一个Linux内核模块示例，演示了字符设备驱动的基本操作
 * 包括：设备注册、ioctl命令的实现、定时器的使用等
 * 该模块实现了一个支持定时器控制的字符设备驱动
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/timer.h>

/* 定义ioctl命令 */
#define TIMER_OPEN _IO('L',0)    // 打开定时器
#define TIMER_CLOSE _IO('L',1)   // 关闭定时器
#define TIMER_SET _IOW('L',2,int)// 设置定时器时间

/* 设备结构体定义 */
struct device_test{
	dev_t dev_num;        // 设备号
	int major;            // 主设备号
	int minor;            // 次设备号
	struct cdev cdev_test;// 字符设备结构体
	struct class *class;  // 设备类
	struct device *device;// 设备结构体
	int counter;          // 定时器计数值
};

/* 定义设备实例 */
struct device_test dev1;

/* 定时器回调函数声明 */
static void function_test(struct timer_list *t);
/* 定义定时器 */
DEFINE_TIMER(timer_test,function_test);

/* 定时器回调函数实现 */
void function_test(struct timer_list *t)
{
	printk("this is function_test\n");
	// 重新设置定时器
	mod_timer(&timer_test,jiffies_64+msecs_to_jiffies(dev1.counter));
}

/* 打开设备函数 */
static int cdev_test_open(struct inode *inode,struct file *file)
{
	file->private_data=&dev1;
	return 0;
}

/* 关闭设备函数 */
static int cdev_test_release(struct inode *inode,struct file *file)
{
	file->private_data=&dev1;
	return 0;
}

/* ioctl命令处理函数 */
static long cdev_test_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct device_test *test_dev=(struct device_test *)file->private_data;

	switch(cmd){
		case TIMER_OPEN:
			// 启动定时器
			add_timer(&timer_test);
			break;
		case TIMER_CLOSE:
			// 停止定时器
			del_timer(&timer_test);
			break;
		case TIMER_SET:
			// 设置定时器时间
			test_dev->counter=arg;
			timer_test.expires=jiffies_64+msecs_to_jiffies(test_dev->counter);
			break;
		default:
			break;
	}
	return 0;
}

/* 文件操作结构体 */
struct file_operations cdev_test_fops={
	.owner=THIS_MODULE,
	.open=cdev_test_open,
	.release=cdev_test_release,
	.unlocked_ioctl=cdev_test_ioctl,
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

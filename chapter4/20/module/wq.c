/*
 * 这是一个Linux内核模块示例，演示了字符设备驱动的基本操作
 * 包括：设备注册、文件操作、等待队列的使用等
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/delay.h>
#include<linux/io.h>
#include<linux/wait.h>

/* 设备结构体定义 */
struct device_test{
	dev_t dev_num;        // 设备号
	int major;            // 主设备号
	int minor;            // 次设备号
	struct cdev cdev_test;// 字符设备结构体
	struct class *class;  // 设备类
	struct device *device;// 设备结构体
	char kbuf[32];        // 内核缓冲区
	int flag;             // 标志位，用于等待队列
};

/* 定义设备实例 */
struct device_test dev1;

/* 声明等待队列头 */
DECLARE_WAIT_QUEUE_HEAD(read_wq);

/* 打开设备函数 */
static int cdev_test_open(struct inode *inode,struct file *file)
{
	file->private_data=&dev1;
	printk("\nThis is open_test\r\n");
	return 0;
}

/* 读设备函数 */
static ssize_t cdev_test_read(struct file *file,char __user *buf,size_t size,loff_t *off)
{
	struct device_test *test_dev=(struct device_test *)file->private_data;
	// 等待直到flag被设置为1
	wait_event_interruptible(read_wq,test_dev->flag);
	// 将内核数据复制到用户空间
	if(copy_to_user(buf,test_dev->kbuf,strlen(test_dev->kbuf))!=0)
	{
		printk("copy_to_user error\n");
		return -1;
	}
	return 0;
}

/* 写设备函数 */
static ssize_t cdev_test_write(struct file *file,const char __user *buf,size_t size,loff_t *off)
{
	struct device_test *test_dev=(struct device_test *)file->private_data;
	int ret;
	// 将用户空间数据复制到内核空间
	ret=copy_from_user(test_dev->kbuf,buf,size);

	if(ret!=0)
	{
		printk("copy_from_user is error\n");
		return -1;
	}

	test_dev->flag=1;
	// 唤醒等待队列中的进程
	wake_up_interruptible(&read_wq);
	return 0;
}

/* 关闭设备函数 */
static int cdev_test_release(struct inode *inode,struct file *file)
{
	return 0;
}

/* 文件操作结构体 */
struct file_operations cdev_test_fops={
	.owner=THIS_MODULE,
	.open=cdev_test_open,
	.read=cdev_test_read,
	.write=cdev_test_write,
	.release=cdev_test_release,
};

/* 模块初始化函数 */
static int __init chr_fops_init(void)
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
static void __exit chr_fops_exit(void)
{
	// 清理设备相关资源
	device_destroy(dev1.class,dev1.dev_num);
	class_destroy(dev1.class);
	cdev_del(&dev1.cdev_test);
	unregister_chrdev_region(dev1.dev_num,1);
	printk("module exit\n");
}

// 模块入口和出口
module_init(chr_fops_init);
module_exit(chr_fops_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

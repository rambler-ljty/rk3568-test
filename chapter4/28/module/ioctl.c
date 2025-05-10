/*
 * 这是一个Linux内核模块示例，演示了字符设备驱动的基本操作
 * 包括：设备注册、ioctl命令的实现等
 * 该模块实现了一个简单的字符设备，支持通过ioctl传递结构体参数
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>

/* 定义ioctl命令 */
#define CMD_TEST0 _IOW('L',0,int)  // 写命令，用于传递结构体参数

/* 定义参数结构体 */
struct args{
	int a;    // 参数a
	int b;    // 参数b
	int c;    // 参数c
};

/* 设备结构体定义 */
struct device_test{
	dev_t dev_num;        // 设备号
	int major;            // 主设备号
	int minor;            // 次设备号
	struct cdev cdev_test;// 字符设备结构体
	struct class *class;  // 设备类
	struct device *device;// 设备结构体
};

/* 定义设备实例 */
struct device_test dev1;

/* ioctl命令处理函数 */
static long cdev_test_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct args test;
	switch(cmd){
		case CMD_TEST0:
			// 从用户空间复制结构体数据到内核空间
			if(copy_from_user(&test,(int *)arg,sizeof(test))!=0){
				printk("copy_from_user error\n");
			}
			// 打印接收到的参数值
			printk("a= %d\n",test.a);
			printk("b= %d\n",test.b);
			printk("c= %d\n",test.c);
			break;
		default:
			break;
	}
	return 0;
}

/* 文件操作结构体 */
struct file_operations cdev_test_fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=cdev_test_ioctl,  // 注册ioctl操作
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

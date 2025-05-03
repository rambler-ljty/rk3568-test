#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/delay.h>
#include<linux/errno.h>
#include<linux/mutex.h>

// 定义互斥锁，用于设备互斥访问控制
struct mutex mutex_test;

/**
 * @brief 设备打开函数
 * @param inode 设备文件的inode结构
 * @param file 设备文件结构
 * @return 成功返回0
 */
static int open_test(struct inode *inode,struct file *file)
{
	printk("\nThis is open_test\n");
	mutex_lock(&mutex_test);  // 获取互斥锁
	return 0;
}

/**
 * @brief 设备读取函数
 * @param file 设备文件结构
 * @param ubuf 用户空间缓冲区
 * @param len 要读取的长度
 * @param off 文件偏移量
 * @return 成功返回0，失败返回-1
 */
static ssize_t read_test(struct file *file,char __user *ubuf,size_t len,loff_t *off)
{
	int ret;
	char kbuf[10]="topeet";  // 内核空间缓冲区
	printk("\nthis is read_test\n");
	ret=copy_to_user(ubuf,kbuf,strlen(kbuf));  // 将数据从内核空间复制到用户空间
	if(ret!=0)
	{
		printk("copy_to_user error\n");
		return -1;
	}

	printk("copy to user is ok\n");
	return 0;
}

static char kbuf[10]={0};  // 内核空间写缓冲区

/**
 * @brief 设备写入函数
 * @param file 设备文件结构
 * @param ubuf 用户空间缓冲区
 * @param len 要写入的长度
 * @param off 文件偏移量
 * @return 成功返回0，失败返回-1
 */
static ssize_t write_test(struct file *file,const char __user *ubuf,size_t len,loff_t *off)
{
	int ret;
	ret=copy_from_user(kbuf,ubuf,len);  // 将数据从用户空间复制到内核空间

	if(ret!=0)
	{
		printk("copy_from_user is error\n");
	}

	// 根据写入的字符串执行不同的延时操作
	if(strcmp(kbuf,"topeet")==0)
	{
		ssleep(4);  // 延时4秒
	}
	else if(strcmp(kbuf,"itop")==0)
	{
		ssleep(2);  // 延时2秒
	}

	printk("copy_from_user buf is %s\n",kbuf);
	return 0;
}

/**
 * @brief 设备关闭函数
 * @param inode 设备文件的inode结构
 * @param file 设备文件结构
 * @return 成功返回0
 */
static int release_test(struct inode *inode,struct file *file)
{
	mutex_unlock(&mutex_test);  // 释放互斥锁
	printk("\nThis is release_test \n");
	return 0;
}

/**
 * @brief 字符设备结构体
 */
struct chrdev_test{
	dev_t dev_num;  //设备号
	int major;      //主设备号
	int minor;      //次设备号
	struct cdev cdev_test;  //字符设备结构
	struct class *class_test;  //设备类
};

struct chrdev_test dev1;  // 全局设备结构体实例

// 文件操作结构体
struct file_operations fops_test={
	.owner=THIS_MODULE,
	.open=open_test,
	.read=read_test,
	.write=write_test,
	.release=release_test,
};

/**
 * @brief 模块初始化函数
 * @return 成功返回0，失败返回错误码
 */
static int __init atomic_init(void)
{
	// 初始化互斥锁
	mutex_init(&mutex_test);
	
	// 动态分配设备号
	if(alloc_chrdev_region(&dev1.dev_num,0,1,"chrdev_name")<0){
		printk("alloc_chrdev_region is error\n");
	}
	printk("alloc_chrdev_region is ok\n");
	dev1.major=MAJOR(dev1.dev_num);
	dev1.minor=MINOR(dev1.dev_num);	
	printk("major is %d\n",dev1.major);
	printk("minor is %d\n",dev1.minor);

	// 初始化字符设备
	cdev_init(&dev1.cdev_test,&fops_test);
	dev1.cdev_test.owner=THIS_MODULE;

	// 添加字符设备
	cdev_add(&dev1.cdev_test,dev1.dev_num,1);

	// 创建设备类
	dev1.class_test=class_create(THIS_MODULE,"class_test");
	
	// 创建设备节点
	device_create(dev1.class_test,0,dev1.dev_num,0,"device_test");

	return 0;
}

/**
 * @brief 模块退出函数
 */
static void __exit atomic_exit(void)
{
	// 清理设备资源
	device_destroy(dev1.class_test,dev1.dev_num);
	class_destroy(dev1.class_test);
	cdev_del(&dev1.cdev_test);
	unregister_chrdev_region(dev1.dev_num,1);
	printk("module exit\n");
}

// 模块入口和出口
module_init(atomic_init);
module_exit(atomic_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("topeet");

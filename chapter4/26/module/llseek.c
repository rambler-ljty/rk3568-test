/*
 * 这是一个Linux内核模块示例，演示了字符设备驱动的基本操作
 * 包括：设备注册、文件操作、文件定位（llseek）功能等
 * 该模块实现了一个简单的字符设备，支持读写和文件定位操作
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/atomic.h>

/* 定义缓冲区大小 */
#define BUFSIZE 1024

/* 定义设备内存缓冲区 */
static char mem[BUFSIZE]={0};

/* 设备结构体定义 */
struct device_test{
	dev_t dev_num;        // 设备号
	int major;            // 主设备号
	int minor;            // 次设备号
	struct cdev cdev_test; // 字符设备结构体
	struct class *class;   // 设备类
	struct device *device;  // 设备结构体
	char kbuf[32];         // 内核缓冲区
};

/* 定义设备实例 */
struct device_test dev1;

/* 打开设备函数 */
static int cdev_test_open(struct inode *inode,struct file *file)
{
	// 设置私有数据
	file->private_data=&dev1;
	return 0;
}

/* 读设备函数 */
static ssize_t cdev_test_read(struct file *file,char __user *buf,size_t size,loff_t *off)
{
	loff_t p=*off;        // 当前文件偏移量
	int i;
	size_t count=size;    // 要读取的数据大小

	// 检查偏移量是否超出缓冲区范围
	if(p>BUFSIZE)
	{
		return -1;
	}

	// 调整读取大小，确保不会超出缓冲区
	if(count>BUFSIZE-p)
	{
		count = BUFSIZE-p;
	}

	// 将数据从内核空间复制到用户空间
	if(copy_to_user(buf,mem+p,count)){
		printk("copy_to_user error\n");
		return -1;
	}

	// 打印调试信息
	for(i=0;i<20;i++){
		printk("buf[%d] is %c\n",i,mem[i]);
	}

	printk("mem is %s, p is %llu,count is %ld\n",mem+p,p,count);
	// 更新文件偏移量
	*off=*off+count;

	return count;
}

/* 写设备函数 */
static ssize_t cdev_test_write(struct file *file,const char __user *buf,size_t size,loff_t *off)
{
	loff_t p=*off;        // 当前文件偏移量
	size_t count=size;    // 要写入的数据大小

	// 检查偏移量是否超出缓冲区范围
	if(p>BUFSIZE){
		return 0;
	}
		
	// 调整写入大小，确保不会超出缓冲区
	if(count>BUFSIZE-p){
		count=BUFSIZE-p;
	}

	// 将数据从用户空间复制到内核空间
	if(copy_from_user(mem+p,buf,count))
	{
		printk("copy_to_user error\n");
		return -1;
	}

	printk("mem is %s,p is %llu\n",mem+p,p);
	// 更新文件偏移量
	*off=*off+count;
	return count;
}

/* 关闭设备函数 */
static int cdev_test_release(struct inode *inode,struct file *file)
{
	return 0;
}

/* 文件定位函数 */
static loff_t cdev_test_llseek(struct file *file,loff_t offset,int whence)
{
	loff_t new_offset;
	switch(whence)
	{
		case SEEK_SET:    // 从文件开始位置定位
			if(offset<0){
				return -EINVAL;
				break;
			}
			if(offset>BUFSIZE){
				return -EINVAL;
				break;    
			}
			new_offset=offset;
			break;
		case SEEK_CUR:    // 从当前位置定位
			if(file->f_pos+offset>BUFSIZE){
				return -EINVAL;
				break;
			}
			if(file->f_pos+offset<0){
				return -EINVAL;
				break;    
			}
			new_offset=file->f_pos+offset;
			break;
		case SEEK_END:    // 从文件末尾定位
			if(file->f_pos+offset<0){
				return -EINVAL;
				break;
			}
			new_offset=BUFSIZE+offset;
			break;
		default:
			break;
	}

	// 更新文件位置
	file->f_pos=new_offset;
	return new_offset;    
}

/* 文件操作结构体 */
struct file_operations cdev_test_fops={
	.owner=THIS_MODULE,
	.open=cdev_test_open,
	.read=cdev_test_read,
	.write=cdev_test_write,
	.release=cdev_test_release,
	.llseek=cdev_test_llseek,  // 添加文件定位操作
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

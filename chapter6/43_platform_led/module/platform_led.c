/* 包含必要的头文件 */
#include<linux/module.h>    /* 提供模块相关的功能 */
#include<linux/init.h>      /* 提供模块初始化和退出相关的宏 */
#include<linux/fs.h>        /* 提供文件操作相关的结构体和函数 */
#include<linux/kdev_t.h>    /* 提供设备号相关的宏和函数 */
#include<linux/cdev.h>      /* 提供字符设备相关的结构体和函数 */
#include<linux/uaccess.h>   /* 提供用户空间和内核空间数据传输的函数 */
#include<linux/io.h>        /* 提供IO内存映射相关的函数 */
#include<linux/platform_device.h>  /* 提供平台设备驱动相关接口 */
#include<linux/ioport.h>           /* 提供IO资源管理相关接口 */

/* 定义设备私有数据结构
 * 用于存储设备相关的所有信息
 */
struct device_test {
	dev_t dev_num;              /* 设备号 */
	int major;                  /* 主设备号 */
	int minor;                  /* 次设备号 */
	struct cdev cdev_test;      /* 字符设备结构体 */
	struct class *class;        /* 设备类 */
	struct device *device;      /* 设备 */
	char kbuf[32];             /* 内核缓冲区，用于存储用户数据 */
	unsigned int *vir_gpio_dr;  /* GPIO数据寄存器的虚拟地址 */
};

/* 声明全局设备结构体变量 */
struct device_test dev1;

/* 设备打开函数
 * 当用户空间调用open()时触发
 * @inode: 设备文件的inode节点
 * @file: 设备文件结构体
 * 返回值: 成功返回0，失败返回负值
 */
static int cdev_test_open(struct inode *inode, struct file *file)
{
	file->private_data = &dev1;  /* 保存设备结构体指针到文件私有数据 */
	printk("This is cdev_test_open\n");
	return 0;
}

/* 设备写入函数
 * 当用户空间调用write()时触发
 * @file: 设备文件结构体
 * @buf: 用户空间数据缓冲区
 * @size: 要写入的数据大小
 * @off: 文件偏移量
 * 返回值: 成功返回写入的字节数，失败返回负值
 */
static ssize_t cdev_test_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	struct device_test *test_dev = (struct device_test *)file->private_data;

	/* 从用户空间复制数据到设备的内核缓冲区 */
	if(copy_from_user(test_dev->kbuf, buf, size) != 0)
	{
		printk("copy_from_user error\n");
		return -1;
	}

	/* 根据用户输入控制LED状态 */
	if(test_dev->kbuf[0] == 1) {
		*(test_dev->vir_gpio_dr) = 0x80008040;  /* LED开启：设置GPIO高电平 */
		printk("test_dev->kbuf[0] is %d\n", test_dev->kbuf[0]);
	}
	else if(test_dev->kbuf[0] == 0) {
		*(test_dev->vir_gpio_dr) = 0x80000040;  /* LED关闭：设置GPIO低电平 */
		printk("test_dev->kbuf[0] is %d\n", test_dev->kbuf[0]);
	}

	printk("This is cdev_test_write\n");
	printk("kbuf[0] is %d\n", test_dev->kbuf[0]);
	return 0;
}

/* 设备读取函数
 * 当用户空间调用read()时触发
 * @file: 设备文件结构体
 * @buf: 用户空间数据缓冲区
 * @size: 要读取的数据大小
 * @off: 文件偏移量
 * 返回值: 成功返回读取的字节数，失败返回负值
 */
static ssize_t cdev_test_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
	struct device_test *test_dev = (struct device_test *)file->private_data;
	
	/* 将设备的内核缓冲区数据复制到用户空间 */
	if(copy_to_user(buf, test_dev->kbuf, strlen(test_dev->kbuf)) != 0)
	{
		printk("copy_to_user error\n");
		return -1;
	}
	printk("This is cdev_test_read\n");
	return 0;
}

/* 设备关闭函数
 * 当用户空间调用close()时触发
 * @inode: 设备文件的inode节点
 * @file: 设备文件结构体
 * 返回值: 成功返回0，失败返回负值
 */
static int cdev_test_release(struct inode *inode, struct file *file)
{
	printk("This is cdev_test_release\n");
	return 0;
}

/* 文件操作结构体
 * 定义设备支持的操作函数
 */
struct file_operations cdev_test_fops = {
	.owner = THIS_MODULE,           /* 模块拥有者 */
	.open = cdev_test_open,         /* 打开操作 */
	.read = cdev_test_read,         /* 读取操作 */
	.write = cdev_test_write,       /* 写入操作 */
	.release = cdev_test_release,   /* 关闭操作 */
};

/* 平台设备探测函数
 * 当内核发现匹配的平台设备时调用
 * @pdev: 平台设备结构体指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_driver_probe(struct platform_device *pdev)
{
	struct resource *res_mem;
	int ret;
	/* 获取设备的内存资源 */
	res_mem=platform_get_resource(pdev,IORESOURCE_MEM,0);
	if(!res_mem){
		dev_err(&pdev->dev,"Failed to get memory resource\n");
		return -ENODEV;
	}

	/* 动态分配设备号 */
	ret = alloc_chrdev_region(&dev1.dev_num, 0, 1, "alloc_name");
	if(ret < 0) {
		goto err_chrdev;
	}
	printk("alloc_chrdev_region is ok\n");
	
	/* 获取主次设备号 */
	dev1.major = MAJOR(dev1.dev_num);
	dev1.minor = MINOR(dev1.dev_num);    
	printk("major is %d\n", dev1.major);
	printk("minor is %d\n", dev1.minor);

	/* 初始化字符设备 */
	dev1.cdev_test.owner = THIS_MODULE;
	cdev_init(&dev1.cdev_test, &cdev_test_fops);

	/* 添加字符设备到系统 */
	ret = cdev_add(&dev1.cdev_test, dev1.dev_num, 1);
	if(ret < 0) {
		goto err_chr_add;
	}

	/* 创建设备类 */
	dev1.class = class_create(THIS_MODULE, "test");
	if(IS_ERR(dev1.class)) {
		ret = PTR_ERR(dev1.class);
		goto err_class_create;
	}

	/* 创建设备节点 */
	dev1.device = device_create(dev1.class, NULL, dev1.dev_num, NULL, "test");
	if(IS_ERR(dev1.device)) {
		ret = PTR_ERR(dev1.device);
		goto err_device_create;
	}

	/* 映射GPIO寄存器到虚拟地址空间 */
	dev1.vir_gpio_dr = ioremap(res_mem->start, 4);
	if(IS_ERR(dev1.vir_gpio_dr)) {
		ret = PTR_ERR(dev1.vir_gpio_dr);
		goto err_ioremap;
	}

	return 0;

	/* 错误处理代码块 */
err_ioremap:
	iounmap(dev1.vir_gpio_dr);
err_device_create:
	class_destroy(dev1.class);
err_class_create:
	cdev_del(&dev1.cdev_test);
err_chr_add:
	unregister_chrdev_region(dev1.dev_num, 1);
err_chrdev:
	return ret;
}

/* 平台设备移除函数
 * 当设备被移除或驱动被卸载时调用
 * @pdev: 平台设备结构体指针
 * 返回值: 成功返回0，失败返回负值
 */
static int my_platform_driver_remove(struct platform_device *pdev)
{
	return 0;
}

/* 定义平台驱动结构体 */
static struct platform_driver my_platform_driver={
	.driver={
		.name="my_platform_device",    /* 驱动名称，需要与平台设备名称匹配 */
		.owner=THIS_MODULE,           /* 模块所有者 */
	},
	.probe=my_platform_driver_probe,      /* 设备探测函数 */
	.remove=my_platform_driver_remove,    /* 设备移除函数 */
};

/* 模块初始化函数
 * 在模块加载时调用
 * 返回值: 成功返回0，失败返回负值
 */
static int __init my_platform_driver_init(void)
{
	int ret;
	/* 注册平台驱动 */
	ret=platform_driver_register(&my_platform_driver);
	if(ret){
		printk("Failed to register platform driver\n");
		return ret;
	}
	
	printk("Platform driver registered\n");
	return 0;
}

/* 模块退出函数
 * 在模块卸载时调用
 */
static void __exit my_platform_driver_exit(void)
{
	device_destroy(dev1.class, dev1.dev_num);   /* 销毁设备节点 */
	class_destroy(dev1.class);                  /* 销毁设备类 */
	cdev_del(&dev1.cdev_test);                  /* 删除字符设备 */
	unregister_chrdev_region(dev1.dev_num, 1);  /* 注销设备号 */

	platform_driver_unregister(&my_platform_driver);
	printk("Platform driver unregistered\n");
}

/* 声明模块的入口和出口函数 */
module_init(my_platform_driver_init);
module_exit(my_platform_driver_exit);

/* 模块许可证声明 */
MODULE_LICENSE("GPL");
/* 模块作者声明 */
MODULE_AUTHOR("topeet");

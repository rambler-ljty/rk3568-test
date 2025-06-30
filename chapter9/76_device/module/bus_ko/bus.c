#include<linux/module.h>    // 提供模块相关的功能
#include<linux/init.h>      // 提供模块初始化和退出函数
#include<linux/slab.h>      // 提供内存分配函数
#include<linux/configfs.h>  // 提供configfs相关接口
#include<linux/kernel.h>
#include<linux/kobject.h>
#include<linux/device.h>
#include<linux/sysfs.h>

int mybus_match(struct device *dev,struct device_driver *drv)
{
	return(strcmp(dev_name(dev),drv->name)==0);
}

int mybus_probe(struct device *dev)
{
	struct device_driver *drv = dev->driver;
	if(drv->probe)
		drv->probe(dev);
	return 0;
}

struct bus_type mybus={
	.name="mybus",
	.match=mybus_match,
	.probe=mybus_probe,

};

EXPORT_SYMBOL_GPL(mybus);

ssize_t mybus_show(struct bus_type *bus,char *buf)
{
	return sprintf(buf,"%s\n","mybus_show");
};

struct bus_attribute mybus_attr={
	.attr = {
		.name ="value",
		.mode =0664,
	},
	.show =mybus_show,
};


static int bus_init(void)
{
	int ret;
	ret=bus_register(&mybus);
	ret=bus_create_file(&mybus,&mybus_attr);
	return 0;

}

static void bus_exit(void)
{
	bus_remove_file(&mybus,&mybus_attr);
	bus_unregister(&mybus);

}


module_init(bus_init);    // 注册初始化函数
module_exit(bus_exit);    // 注册清理函数

MODULE_LICENSE("GPL");           // 模块许可证
MODULE_AUTHOR("topeet");         // 模块作者

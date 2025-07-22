#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/gpio/consumer.h>

// GPIO 描述符指针
struct gpio_desc *mygpio1;
struct gpio_desc *mygpio2;
// GPIO 编号
int num;

/**
 * 平台设备初始化函数
 * @param dev 平台设备指针
 * @return 0成功，负值表示失败
 */
static int my_platform_probe(struct platform_device *dev)
{
    printk("This is my_platform_probe\n");

    // 获取可选的GPIO描述符
    mygpio1 = gpiod_get_optional(&dev->dev, "my", 0);
    if (mygpio1 == NULL) {
        printk("gpiod_get_optional error\n");
        return -1;
    }
    num = desc_to_gpio(mygpio1);
    printk("num is %d\n", num);

    // 释放GPIO描述符
    gpiod_put(mygpio1);

    // 获取指定索引的GPIO描述符
    mygpio2 = gpiod_get_index(&dev->dev, "my", 0, 0);
    if (IS_ERR(mygpio2)) {
        printk("gpiod_get_index error\n");
        return -2;
    }
    num = desc_to_gpio(mygpio2);
    printk("num is %d\n", num);

    return 0;
}

/**
 * 平台设备的移除函数
 * @param pdev 平台设备指针
 * @return 0成功
 */
static int my_platform_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "my_platform_remove: Removing platform device\n");

    // 清理设备特定的操作
    // ...

    return 0;
}

// 设备匹配表
const struct of_device_id of_match_table_id[]  = {
	{.compatible="mygpio"},
};

// 定义平台驱动结构体
static struct platform_driver my_platform_driver = {
    .probe = my_platform_probe,
    .remove = my_platform_remove,
    .driver = {
        .name = "my_platform_device",
        .owner = THIS_MODULE,
		.of_match_table =  of_match_table_id,
    },
};

// 模块初始化函数，注册平台驱动
static int __init my_platform_driver_init(void)
{
    int ret;

    ret = platform_driver_register(&my_platform_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register platform driver\n");
        return ret;
    }

    printk(KERN_INFO "my_platform_driver: Platform driver initialized\n");

    return 0;
}

// 模块退出函数，注销平台驱动
static void __exit my_platform_driver_exit(void)
{
    gpiod_put(mygpio2);
    platform_driver_unregister(&my_platform_driver);

    printk(KERN_INFO "my_platform_driver: Platform driver exited\n");
}

module_init(my_platform_driver_init);
module_exit(my_platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("topeet");

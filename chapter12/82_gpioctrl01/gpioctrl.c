#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// 文件描述符
int fd;
// 写操作返回值
int ret;
// GPIO路径字符串
char gpio_path[100];
// 字符串长度
int len;

/**
 * 导出指定的GPIO，使其可被用户空间访问
 * @param argv GPIO编号字符串
 * @return 0成功，负值表示失败
 */
int gpio_export(char *argv)
{
	fd=open("/sys/class/gpio/export",O_WRONLY);
	if(fd<0)
	{
		printf("open /sys/class/gpio/export error\n");
		return -1;
	}
	len=strlen(argv);
	ret=write(fd,argv,len);

	if(ret<0)
	{
		printf("write /sys/class/gpio/export error\n");
		return -2;
	}
	close(fd);
	return 0;
}

/**
 * 取消导出指定的GPIO，释放资源
 * @param argv GPIO编号字符串
 * @return 0成功，负值表示失败
 */
int gpio_unexport(char *argv)
{
	fd=open("/sys/class/gpio/unexport",O_WRONLY);
	if(fd<0)
	{
		printf("open /sys/class/gpio/unexport error\n");
		return -1;
	}

	len=strlen(argv);
	ret=write(fd,argv,len);
	if(ret<0)
	{
		printf("write /sys/class/gpio/unexport error \n");
		return -2;
	}

	close(fd);
	return 0;
}


/**
 * 控制GPIO的属性，如方向或电平值
 * @param arg 属性名称，如"direction"或"value"
 * @param val 属性值，如"out"或"1"
 * @return 0成功，负值表示失败
 */
int gpio_ctrl(char *arg,char *val)
{
	char file_path[100];
	// 拼接GPIO属性文件路径
	sprintf(file_path,"%s/%s",gpio_path,arg);
	fd=open(file_path,O_WRONLY);
	if(fd<0)
	{
		printf("open file_path error \n");
		return -1;
	}
	len=strlen(val);
	ret=write(fd,val,len);
	if(ret<0)
	{
		printf("write file_path error\n");
		return -2;
	}
	close(fd);
	return 0;
}


/**
 * 主函数，控制GPIO的导出、设置方向和值，最后取消导出
 * @param argc 参数个数
 * @param argv 参数数组，argv[1]为GPIO编号，argv[2]为输出值
 * @return 0程序正常结束
 */
int main(int argc,char *argv[])
{
	// 设置GPIO路径
	sprintf(gpio_path,"/sys/class/gpio/gpio%s",argv[1]);
	// 判断GPIO路径是否存在，若不存在则导出GPIO
	if(access(gpio_path,F_OK))
	{
		gpio_export(argv[1]);
	}
	else
	{
		// 若存在则取消导出GPIO
		gpio_unexport(argv[1]);
	}

	// 设置GPIO方向为输出
	gpio_ctrl("direction","out");
	// 设置GPIO输出值
	gpio_ctrl("value",argv[2]);

	// 取消导出GPIO，释放资源
	gpio_unexport(argv[1]);
	
	return 0;

}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

// GPIO寄存器基地址
#define GPIO_REG_BASE 0XFDD60000
// GPIO端口方向寄存器偏移
#define GPIO_SWPORT_DDR_L_OFFSET 0x0008
// GPIO端口数据寄存器偏移
#define GPIO_SWPORT_DR_L_OFFSET 0x0000
// 映射大小
#define SIZE_MAP 0X1000


/**
 * 打开LED灯
 * @param base 映射的GPIO寄存器基地址
 */
void LED_ON(unsigned char *base)
{
	// 设置GPIO端口方向寄存器，配置为输出
	*(volatile unsigned int *)(base+GPIO_SWPORT_DDR_L_OFFSET)=0X8000C044;

	// 设置GPIO端口数据寄存器，输出高电平，点亮LED
	*(volatile unsigned int *)(base+GPIO_SWPORT_DR_L_OFFSET)=0X8000C044;
}

/**
 * 关闭LED灯
 * @param base 映射的GPIO寄存器基地址
 */
void LED_OFF(unsigned char *base)
{
	// 设置GPIO端口方向寄存器，配置为输出
	*(volatile unsigned int *)(base+GPIO_SWPORT_DDR_L_OFFSET)=0X8000C044;

	// 设置GPIO端口数据寄存器，输出低电平，关闭LED
	*(volatile unsigned int *)(base+GPIO_SWPORT_DR_L_OFFSET)=0X80004044;
}

/**
 * 主函数，映射GPIO寄存器地址，循环控制LED灯闪烁
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 0程序正常结束，负值表示错误
 */
int main(int argc,char *argv[])
{
	int fd;

	unsigned char *map_base;

	fd=open("/dev/mem",O_RDWR);

	if(fd<0)
	{
		printf("open /dev/mem error\n");
		return -1;
	}

	map_base=(unsigned char *)mmap(NULL,SIZE_MAP,PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO_REG_BASE);

	if(map_base==MAP_FAILED)
	{
		printf("map_base error\n");
		return -2;
	
	}	

	while(1)
	{
		LED_ON(map_base);
		usleep(1000000);

		LED_OFF(map_base);
		usleep(1000000);
	
	}

	munmap(map_base,SIZE_MAP);

	close(fd);

	return 0;

}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

// 文件描述符
int fd;
// 写操作返回值
int ret;
// GPIO路径字符串
char gpio_path[100];
// 字符串长度
int len;
// GPIO属性文件路径
char file_path[100];
// 读取缓冲区
char buf[2];
// poll结构体数组
struct pollfd fds[1];

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
 * 控制GPIO的属性，如方向或边缘触发
 * @param arg 属性名称，如"direction"或"edge"
 * @param val 属性值，如"in"或"both"
 * @return 0成功，负值表示失败
 */
int gpio_ctrl(char *arg,char *val)
{
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
 * 处理中断事件，使用poll监听GPIO值变化
 * @param arg 属性名称，通常为"value"
 * @return 0成功，负值表示失败
 */
int gpio_interrupt(char *arg)
{
	sprintf(file_path,"%s/%s",gpio_path,arg);
	fd=open(file_path,O_WRONLY);
	if(fd<0)
	{
		printf("open file_path error\n");
		return -1;
	}
	memset((void *)fds,0,sizeof(fds));
	fds[0].fd=fd;
	fds[0].events=POLLPRI;
	read(fd,buf,2);

	ret=poll(fds,1,-1);

	if(ret<=0)
	{
		printf("poll error\n");
		return -1;
	}

	if(fds[0].revents & POLLPRI)
	{
		lseek(fd,0,SEEK_SET);
		read(fd,buf,2);
		buf[1]='\0';
		printf("value is %s\n",buf);
	}
	return 0;
}

/**
 * 读取GPIO的电平值
 * @param arg 属性名称，通常为"value"
 * @return 1高电平，0低电平，-1错误
 */
int gpio_read_value(char *arg)
{
	sprintf(file_path,"%s/%s",gpio_path,arg);
	fd=open(file_path,O_WRONLY);
	if(fd<0)
	{
		printf("open file_path error\n");
		return -1;
	}
	ret=read(fd,buf,1);
	if(!strcmp(buf,"1"))
	{
		printf("The value is high\n");
		return 1;
	}
	else if(!strcmp(buf,"0"))
	{
		printf("The value is low\n");
		return 0;
	}

	return -1;
	close(fd);

}



int main(int argc,char *argv[])
{
	int value;
	sprintf(gpio_path,"/sys/class/gpio/gpio%s",argv[1]);
	if(access(gpio_path,F_OK))
	{
		gpio_export(argv[1]);
	}
	else
	{
		gpio_unexport(argv[1]);
	}

	gpio_ctrl("direction","in");
	gpio_ctrl("edge","both");
	gpio_interrupt("value");

	gpio_unexport(argv[1]);
	
	return 0;

}

/*
 * 这是一个测试程序，用于测试字符设备驱动的异步通知功能
 * 该程序通过信号机制实现异步通知，当设备有数据时触发SIGIO信号
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>

int fd;                  // 文件描述符
char buf1[32] = {0};     // 读取缓冲区

/* SIGIO信号的信号处理函数 */
static void func(int signum)
{
    read(fd,buf1,32);    // 读取设备数据
    printf ("buf is %s\n",buf1);  // 打印读取的数据
}

int main(int argc, char *argv[])  
{
    int ret;
    int flags;

    // 打开设备节点
    fd = open("/dev/test", O_RDWR);
    if (fd < 0)
    {
        perror("open error \n");
        return fd;
    }

    // 步骤一：注册SIGIO信号的信号处理函数
    signal(SIGIO,func);

    // 步骤二：设置进程接收SIGIO信号
    fcntl(fd,F_SETOWN,getpid());

    // 步骤三：获取文件描述符标志
    flags = fcntl(fd,F_GETFD);

    // 步骤四：设置文件描述符支持异步通知
    fcntl(fd,F_SETFL,flags| FASYNC);

    // 主循环，等待信号
    while(1);

    // 关闭设备
    close(fd);
    return 0;
}

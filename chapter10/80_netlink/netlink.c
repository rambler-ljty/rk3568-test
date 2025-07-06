#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<linux/netlink.h>

// 主函数入口
int main(int argc,char *argv[]){
    int ret;
    struct sockaddr_nl *nl;
    int len=0;
    char buf[4096]={0};
    int i=0;

    // 分配并清零 struct sockaddr_nl 结构体空间
    bzero(nl,sizeof(struct sockaddr_nl));
    // 设置协议族为 AF_NETLINK
    nl->nl_family=AF_NETLINK;
    // 设置接收所有内核消息
    nl->nl_pid=0;
    nl->nl_groups=1;

    // 创建 netlink socket，协议为 NETLINK_KOBJECT_UEVENT
    int socket_fd=socket(AF_NETLINK,SOCK_RAW,NETLINK_KOBJECT_UEVENT);

    if(socket_fd<0){
        printf("socket error\n");
        return -1;
    }

    // 绑定 socket 到 netlink 地址
    ret=bind(socket_fd,(struct sockaddr *)nl,sizeof(struct sockaddr_nl));

    if(ret<0){
        printf("bind error\n");
        return -1;
    }

    // 循环接收内核消息
    while(1){
        bzero(buf,4096); // 清空缓冲区
        len=recv(socket_fd,&buf,4096,0); // 接收消息

        // 将消息中的 '\0' 替换为换行符，便于打印
        for(i=0;i<len;i++){
            if(*(buf+i)=='\0'){
                buf[i]='\n';
            }
        }

        printf("%s\n",buf); // 打印消息
    }

    return 0;
}

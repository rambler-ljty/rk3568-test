#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<poll.h>
#include<fcntl.h>
#include<signal.h>

int fd;
char buf1[32]={0};


static void func(int signum)
{
	read(fd,buf1,32);
	printf("buf is %s\n",buf1);
}


int main(int argc,char *argv[])
{
	int ret;
	int flags;

	fd=open("/dev/test",O_RDWR);

	if(fd<0){
		perror("file open error\n");
		return fd;
	}


//	signal(SIGIO,func);
	signal(SIGIO,func);
//	fcntl(fd,F_SETOWN,getpid());
	fcntl(fd,F_SETOWN,getpid());
//	flags = fcntl(fd,F_GETFD);
	flags = fcntl(fd,F_GETFD);
//	fcntl(fd,F_SETFL,flags| FASYNC);
	fcntl(fd,F_SETFL,flags| FASYNC);

	while(1)

	close(fd);


	return 0;
}

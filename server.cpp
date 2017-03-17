#include<stdio.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<string.h>
#include<errno.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>

#define IPADDRESS "127.0.0.1"
#define PORT 6666
#define LISTENQ 5
#define MAXSIZE 1024
#define EPOLLEVENTE 100 
#define FDSIZE 1000
int socket_bind(const char *ip,int port)
{
	int listenfd;
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if(listenfd<0)
	{
		perror("listen:");
		exit(1);
	}
	struct sockaddr_in myserver;
	memset(&myserver,0,sizeof(myserver));

	myserver.sin_family=AF_INET;
	myserver.sin_port=htons(port);
	inet_pton(AF_INET,ip,&myserver.sin_addr);
	if((bind(listenfd,(struct sockaddr *)&myserver,sizeof(myserver)))==-1)
	{
		perror("bind error:" );
		exit(1);
	}
	return listenfd;

}
void delete_event(int epfd,int fd,int state)
{
	struct epoll_event event;
	event.events=state;
	event.data.fd=fd;
	epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&event);

}
void add_event(int epfd,int fd,int state)
{
	struct epoll_event event;
	event.events=state;
	event.data.fd=fd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
}
void handle_accept(int epfd,int listenfd)
{
	int acceptfd;
	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	acceptfd=accept(listenfd,(struct sockaddr *)&clientaddr,&clientlen);
	if(acceptfd==-1)
	{
		perror("accept:");
		exit(1);
	}
	else
	{
		printf("accept a new client :%s:%d\n",inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);
		add_event(epfd,acceptfd,EPOLLIN);
	}
}
void modify_event(int epfd,int fd,int state)
{
	struct epoll_event ev;
	ev.events=state;
	ev.data.fd=fd;
	epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
}

void do_read(int epfd,int fd,char *buf)
{
	int nread=read(fd,buf,MAXSIZE);
	if(nread==-1)
	{
		perror("read error:");
		close(fd);
		delete_event(epfd,fd,EPOLLIN);

	}
	else if(nread==0)
	{
		fprintf(stderr,"client close.\n");
		close(fd);
		delete_event(epfd,fd,EPOLLIN);
	}
	else{
		printf(" -----------------------------------testing _____________________--------------------------------");
		printf("read msg is  :%s ",buf);
		modify_event(epfd,fd,EPOLLOUT);
	}
}
void do_write(int epfd,int fd,char *buf)
{
	int nwrite;
	nwrite=write(fd,buf,strlen(buf));
	if(nwrite==-1)
	{
		perror("write error:");
		close(fd);
		delete_event(epfd,fd,EPOLLOUT);
	}
	else 
	{
		modify_event(epfd,fd,EPOLLIN);

	}
	memset(buf,0,MAXSIZE);

}
void handle_events(int epfd,struct epoll_event *events, int num,int listenfd,char *buf)
{
	int i;
	int fd;
	for(i=0;i<num;i++)
	{
		fd=events[i].data.fd;
		if((fd==listenfd)&&events[i].events&EPOLLIN)
		{
			handle_accept(epfd,listenfd);

		}
		else if(events[i].events&EPOLLIN)
		{
			do_read(epfd,fd,buf);

		}
		else if(events[i].events&EPOLLOUT)
		{
			do_write(epfd,fd,buf);
		}
	}
}
void do_epoll(int listenfd)
{
	int epfd=epoll_create(FDSIZE);
	struct epoll_event event[EPOLLEVENTE];
	char buf[MAXSIZE];
	memset(buf,0,MAXSIZE);
	add_event(epfd,listenfd,EPOLLIN);
	while(1)
	{

		int rv=epoll_wait(epfd,event,EPOLLEVENTE,-1);
		handle_events(epfd,event,rv,listenfd,buf);
	}
	close(epfd);
}
int main()
{
	int listenfd;
	listenfd=socket_bind(IPADDRESS,PORT);
	listen(listenfd,LISTENQ);
	do_epoll(listenfd);
	return 0;
}

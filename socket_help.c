#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* PROD_READ */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> //memcpy所需
#include <sys/socket.h>
#include <netdb.h>
#include "rio.c"

#define LISTENQ 1024 //暗示了内核在开始拒绝连接请求之前，队列中要排队的未完成的连接请求数量
#define MAXLINE 8096
extern char **environ;
/********************************************************************
 *
 *
 *	socket相关的辅助函数，可以直接使得客户端连接上，或者服务端开启监听
 *  ------------------- 未包含main 编译会出现错误 ------------------- 
 *
 ********************************************************************/

//创建客户端连接描述符
//int open_clientfd(char *hostname, char *port);

//创建并返回一个监听描述符(服务端),服务端监听只需要打开相应的监听端口
//int open_listenfd(char *port);

//自定义错误处理函数
void my_error(char *mes)
{
	fprintf(stderr, "%s : %s\n", mes, strerror(errno));
	exit(0);
}


//包含有错误处理的accept()
int Accept(int listenfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int rc;
	char *err = "unix_error";
	if ((rc = accept(listenfd, addr, addrlen)) < 0)
	{
		fprintf(stderr, "%s : %s\n", err, strerror(errno));
		exit(0);
	}
	return rc;
}


//创建并返回客户端连接描述符(可以直接用 unix i/o)
//socket() - connect() - 获得clientfd
int open_clientfd(char *hostname, char *port)
{
	/*    创建变量    */
	int clientfd;
	//hints指向一个addrinfo结构 它提供对套接字地址列表的更好的控制
	struct addrinfo hints, *listp, *p;

	/*    初始化配置参数	*/
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM; //设置成返回连接的addrinfo结构	
	hints.ai_flags = AI_NUMERICSERV; //使用(数字)端口
	//异或掩码
	hints.ai_flags |= AI_ADDRCONFIG;
	
	//获得连接成功的套接字结构
	getaddrinfo(hostname, port, &hints, &listp);
	
	//遍历listp指向的addrinfo结构 获得连接套接字描述符
	for (p = listp; p; p = p->ai_next)
	{
		//创建socket描述符,利用的是getaddrinfo()的形式，目的是协议无关
		if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
			//一直尝试创建，直到创建成功
			continue;
		
		//和服务端连接(clientfd已经创建成功),此时客户端会一直阻塞在connect() 除非服务端连接通成功或者出现连接错误
		if ((connect(clientfd, p->ai_addr, p->ai_addrlen) != -1))
			//连接成功，跳出循环
			break;

		//连接失败
		close(clientfd); //重新连接 重新生成clientfd
	}

	//释放listp
	freeaddrinfo(listp);
	//判断p当前的指向的套接字结构，为空则表示一直没有连接上，
	if (p)
		return clientfd;//连接成功
	else
		return -1; //连接失败
}

//创建并返回一个监听描述符(服务端),服务端监听只需要打开相应的监听端口
//socket() - bind() - listen()
int open_listenfd(char *port)
{
	/*    创建变量    */
	struct addrinfo hints, *listp, *p;	
	int listenfd, optval = 1;

	/*    配置    */
	//初始化hints
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM; //接受连接选项
	//位掩码，设置可以修改默认行为，用or(|) 组合获得掩码
	hints.ai_flags = AI_PASSIVE; // 这个标志告诉返回的套接字地址可能被服务器用作 监听套接字 同时告诉
								 // 内核 这个服务器会接受发送到本主机的所有ip地址的请求
	hints.ai_flags |= AI_ADDRCONFIG; // 使用连接的配置
	hints.ai_flags |= AI_NUMERICSERV; //强制参数service 为端口号
	
	//获得套接字结构
	getaddrinfo(NULL, port, &hints, &listp);//用于监听
	
	//遍历
	for (p = listp; p; p = p->ai_next)
	{
		// 创建一个监听描述符
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			//同上
			continue;
		
		//配置服务器模式
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
		
		//绑定监听描述符
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			//绑定成功
			break; 
		close(listenfd); //关闭，重新监听
	}

	//回收内存
	freeaddrinfo(listp);
	//在遍历listp时，没有一个成功的(socket + binding)
	if (!p) return -1;

	//listen函数将套接字转化为 一个监听套接字，可以接受来自客户端的连接请求，
	//转为监听描述符
	if (listen(listenfd, LISTENQ) < 0)
	{
		//关闭释放listentfd
		close(listenfd);
		return -1;
	}

	//返回监听描述符
	return listenfd;
}

/*    健壮的相关函数	*/
int Open_clientfd(char *host, char *port)
{
	int rc;

	if ((rc = open_clientfd(host, port)) < 0)	
	{
		if (rc == -1)
		{
			my_error("Open_clientfd Linux error");
		}
		else 
		{
			my_error("Open_clientfd DNS error");
		}
		//exit(0);
	}
	return rc;
}

int Open_listenfd(char *port)
{
	int rc;

	if ((rc = open_listenfd( port)) < 0)	
	{
		my_error("Open_clientfd Linux error");
		//exit(0);
	}
	return rc;
}

/**
 *	mserver.c 简单的web服务器，支持HTTTP1.0 GET POST
 *
 *	@author	:	MCX
 *	@email	:	i_allblue@163.com
 */
#include "socket_help.c"

/**
 *	处理事务(web请求)
 */
void doit(int fd);

/**
 * 处理请求行
 */
void read_requesthdrs(my_rio_t *rp, int *length);

/**
 *	解析uri
 */
int parse_uri(char *uri, char *filename, char *cgiargs);

/**
 *	提供静态资源请求服务
 */
void serve_static(int fd, char *filename, int filesize);

/**
 *	提供post请求处理
 */ 
void post_dynamic(int fd, char *filename, my_rio_t *rp, int content_length);	
/**
 *	获得请求资源的类型
 */
void get_filetype(char *filename, char *filetype);

/**
 *	提供动态资源请求服务
 */
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);

/**
 *	错误处理函数
 */
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/**
 *  mserver 入口(main)
 */
int main(int argc, char **argv)
{
	//-------------------------
    //	变量定义
    //-------------------------
	char *listen_port, client_port[MAXLINE], hostname[MAXLINE];
	int listenfd, connfd;
	struct sockaddr_storage clientaddr;	
	socklen_t clientlen;

	//-------------------------
    //	检测命令行参数
    //-------------------------
	if (argc != 2)
	{
		//atoi();
		fprintf(stderr, "usage: %s <port> \n", argv[0]);
		exit(0);
	}
	
	listen_port = argv[1];
	//--------------------------------------------------------------------------
	//获得监听描述符 （思考为什么只需要打开一个监听描述符！！！）
	//因为只需要一个监听即可，一个线程负责监听，然后创建新的进程或线程来执行任务
	//--------------------------------------------------------------------------
	listenfd = Open_listenfd(listen_port);

	int log = open("log.txt", O_RDWR | O_APPEND,0);	
	//dup2(log, STDOUT_FILENO);
	// printf("hello,world\n");
	//不断处理连接请求
	while (1)
	{
		clientlen = sizeof(clientaddr);

		//获得连接套接字描述符
		connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		
		//获得客户端相应信息(参数最后一个为flags = 0 表示采用默认行为)
		getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLINE, client_port, MAXLINE, 0);
		//客户端连接成功
		printf("Accepted connection from (%s, %s)\n", hostname, client_port);
		
		//处理客户端事务(请求)
		doit(connfd);
		
		//关闭连接描述符
		close(connfd);
	}

	// 关闭log
	close(log);
	return 0;
}


/**
 *	处理事务(web请求)
 */
void doit(int fd)
{
	//判断是否是静态资源
	int is_static;
	// 报文长度
	int *p_length, length;
	p_length = &length;
	//文件属性信息
	struct stat sbuf;
	char filename[MAXLINE], cgiargs[MAXLINE];
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	my_rio_t rio;
	
	//--------------------
	//读取请求行和请求头部
	//--------------------
	
	//将rio和连接描述符绑定，直接去处理rio
	my_rio_readinitb(&rio, fd);
	//读取一行---请求行
	my_rio_readlineb(&rio, buf, MAXLINE);
	printf("Request Lines:\n");
	printf("%s", buf);
	// 可以通过env获得参数
	//取得参数
	sscanf(buf, "%s %s %s", method, uri, version);
	
	//暂时支持GET
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		//错误处理
		client_error(fd, method, "501", "Not implemented", "Mserver does not accetpt this method");
		return;
	}
		
	//处理请求头部(注意内部指针有移动)
	printf("Request Headers:\n");
	read_requesthdrs(&rio, p_length);

	//解析uri获得filename cgiargs 同时判断请求的uri是否合法
	is_static = parse_uri(uri, filename, cgiargs);
	
	//判读文件是否存在且可读
	if (stat(filename, &sbuf) < 0)
    { 
 		//说明文件不存在
		client_error(fd, filename, "404", "Not found", "Sorry, Mserver couldn't find this file");
		return;
    }
	if (is_static) /* 静态资源  */
	{
		//--------------------------------
        //	宏调用
   		//S_ISREG() - 这是一个普通文件吗
		//S_ISDIR() - 这是一个目录文件吗
		//S_ISSOCK()- 这是一个网络套接字吗
		//--------------------------------
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode )) /* 判断是否一个普通文件，且有权利读 */
		{
			client_error(fd, filename, "403", "Forbidden", "Mserver couldn't read this file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else
	{	/* 动态内容 */
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) /* 判断是否是一个普通文件且可执行 */
		{
			client_error(fd, filename, "403", "Forbidden", "Mserver couldn't run the CGI program");
			return;
		}
		/* 区分是post请求还是get请求 */
		if (strcasecmp(method, "POST") == 0)
		{	
			post_dynamic(fd, filename, &rio, length);	
		} else {
			serve_dynamic(fd, filename, cgiargs, method);
		}
	}
	/*	
	while ((my_rio_readlineb(&rio, buf, MAXLINE)) != 0)
	{
		printf("%s\n", buf);
	}
	*/
	
	//不知道为什么下面的read会一直阻塞
	//int res = read(fd, buf,2);
	//printf("res = %d  %s\n", res, buf);
		
	
	//send(fd, buf, strlen(buf), 1);
	//发现一个问题假如执行下面这条语句，则客户端无法立刻显示服务端的信息，但是确实已经收到了，就是不显示
	//sleep(100);
}

/**
 * 处理请求头部，简单的跳过请求头部信息，没有具体处理参数
 */
void read_requesthdrs(my_rio_t *rp, int *length)
{
	char buf[MAXLINE], *p, con[100];
	int len = 1;

	my_rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n"))
	{
		//一行一行的读取(也会读取\n)
		my_rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);	

		//获得post提交的Content-Length
		// p = strstr(buf, "Content-Length:");
		// if (p)	/* buf中含有content-length*/
		// {
		// 	p = buf + 15;	
		// 	len = atoi(p); 
		// 	*length = len;
		// 	// *length = 10;
		// }
		if(strncasecmp(buf,"Content-Length:",15)==0)
		{
			p=&buf[15];	
			p+=strspn(p," \t");
			*length=atol(p);
		}
	}
	return;
}



/**
 *	解析uri
 *	默认将动态资源放在cgi-bin目录下
 *	@return  1 : 动态内容   0 ： 静态内容  
 */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;

	//默认请求页面index.html
	if (!strstr(uri, "cgi-bin")) /* 静态资源  */
	{
		strcpy(cgiargs, ""); //静态资源 清空cgiargs
		strcpy(filename, "."); //设当前工作目录为主目录
		strcat(filename, uri); //假如uri是 /index.html 则filename会变成 ./index.html
	
		//设置默认的目录的主页面是index.html
		if (uri[strlen(uri) - 1] == '/')
		{
			strcat(filename, "index.html");
		}
			return 1;
	} else { /* 动态资源 */
		//默认动态资源的工作目录在cgi-bin
		ptr = index(uri, '?'); //目的是为了获取参数
		if (ptr)
		{
			strcpy(cgiargs, ptr + 1);
			//可能是销毁ptr
			*ptr = '\0';
		}	
		else
			strcpy(cgiargs, ""); /* 参数赋空值 */

		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
    }	

}

/**
 *	提供静态资源请求服务
 */
void serve_static(int fd, char *filename, int filesize)
{
	int srcfd; /* 文件描述符 */
	char *srcp; /* 文件映射到内存的指针 */
	char filetype[MAXLINE], buf[MAXLINE];	

	//构造响应头
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Mserver web Server\r\n", buf);
	sprintf(buf, "%sConnection: close\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	//输出信息
	my_writen(fd, buf, strlen(buf));
	printf("Response headers:\n");
	printf("%s", buf);/*	*/

	//向客户端发送信息 利用虚拟内存映射
	srcfd = open(filename, O_RDONLY, 0); /* 只读方式打开请求文件 */	
	// 类似于buffer
	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); /* 直接将文件映射到内存，效率高(系统调用按照页进行映射的)*/
	close(srcfd);
	my_writen(fd, srcp, filesize);
	munmap(srcp, filesize); /* 清空内存，避免内存泄露 */
	
}

/**
 *	获得请求资源的类型
 *	通过文件名称，获得文件类型
 */
void get_filetype(char *filename, char *filetype)
{
	//判断主串中是否包含子串
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".png"))
		strcpy(filetype, "image/png");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else /* 无格式正文 */
		strcpy(filetype,"text/plain");
}

/**
 *	提供动态资源请求服务
 *	创建一个进程，在子进程中执行程序
 */
void serve_dynamic(int fd, char *filename, char *cgiargs, char * method)
{
	char *emptylist[] = {NULL};
	char buf[MAXLINE];
	
	//构造HTTP响应
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	my_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Mserver web server\r\n");
	my_writen(fd, buf, strlen(buf));

	if (fork() == 0) /* 创建子进程 */
	{
		//设置环境变量
		setenv("QUERY_STRING", cgiargs, 1);
		dup2(fd, STDOUT_FILENO); /* 重定向(程序的执行结果会输出到标准输出，然后重定向到连接套接字) */
		execve(filename, emptylist, environ); /* 执行程序 */
	}
	wait(NULL); /* 回收进程 */
}

/**
 *	处理post请求资源
 *	创建一个进程，在子进程中执行程序
 */
void post_dynamic(int fd, char *filename, my_rio_t *rp, int length)
{
	char *emptylist[] = {NULL}, *content_length;
	char buf[MAXLINE], data[MAXLINE];
	int post_pipe[2];	/* 借助管道发送post数据到标准输出 */
	pid_t pid;

	//构造HTTP响应
	// sprintf(buf, "HTTP/1.0 200 OK\r\n");
	// my_writen(fd, buf, strlen(buf));
	// sprintf(buf, "Server: Mserver web server\r\n");
	// my_writen(fd, buf, strlen(buf));

	sprintf(content_length, "%d", length);
	memset(data, 0, MAXLINE);



	//-------------------------------------------
	// 借助tinyhttp的实现，发现post是借助管道进行通讯的
	//-------------------------------------------

	// 获得post数据
	// my_rio_readnb(rp, data, length);	/* 则需要把data数据传递到(子进程)标准输入中 */	
	// printf("%s\n",data);	 /* 获取到post数据 */


	// 创建管道 post_pipe(1:写端 0:读端)
	// 父进程打开post_pipe[1] 写端
	//      关闭post_pipe[0] 读端
	// 古老的管道是半双工的，数据流是单向的
	pipe(post_pipe);

	// close(post_pipe[0]);
	// my_writen(post_pipe[1], data, length);
/*---------------------------为什么不能让父进程读数据 而子进程去执行(难道只有这样才能保证数据读取吗)---------------------------*/
	if ((pid = fork()) == 0) /* 创建子进程 */
	{
		
		close(post_pipe[0]);
      	my_rio_readnb(rp,data,length);  
        my_writen(post_pipe[1],data,length); /* 向管道写端写入post数据 */
        exit(0);
		// dup2(post_pipe[0], STDIN_FILENO);
		// close(post_pipe[0]);
		// content_length = "101";
		// printf("hello world%s\n",content_length);
		// setenv("CONTENT-LENGTH", content_length, 1);
		// dup2(fd, STDOUT_FILENO); /* 重定向(程序的执行结果会输出到标准输出，然后重定向到连接套接字) */
		// execve(filename, emptylist, environ); /* 执行程序 */
	} 
	dup2(post_pipe[0], STDIN_FILENO); /* 管道读端重定向到标准输出 */
	close(post_pipe[0]);
	close(post_pipe[1]);
	setenv("CONTENT-LENGTH", content_length, 1);

	dup2(fd, STDOUT_FILENO); /* */
	execve(filename, emptylist, environ); /* 执行程序 */

	wait(NULL); /* 回收进程 */

}
/**
 *	错误处理函数
 * @fd 连接描述符
 * @cause 资源名称
 * @errnum 错误状态码
 * @shortmsg 错误信息
 * @longmsg 错误提示信息	
 */
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXLINE];
	
	//构造响应正文
	sprintf(body, "<html><title>Mserver Error</title>");
	sprintf(body, "%s<body bgcolor=\"#ffffff\">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum,shortmsg);
	sprintf(body, "%s<p>%s:%s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Mserver web server</em>\r\n", body);

	//输出响应信息
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	//发送信息
	my_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	my_writen(fd, buf, strlen(buf));
	//两个回车换行
	sprintf(buf, "Content-length: %d\r\n\r\n",(int)strlen(body));
	//输出信息
	my_writen(fd, buf, strlen(buf));
	//输出响应内容
	my_writen(fd, body, strlen(body));
}

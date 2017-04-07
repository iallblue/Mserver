/**
 *	mserver.c �򵥵�web��������֧��HTTTP1.0 GET 
 *
 */
#include "socket_help.c"

/**
 *	��������(web����)
 */
void doit(int fd);

/**
 * 
 */
void read_requesthdrs(my_rio_t *rp);

/**
 *	����uri
 */
int parse_uri(char *uri, char *filename, char *cgiargs);

/**
 *	�ṩ��̬��Դ�������
 */
void serve_static(int fd, char *filename, int filesize);

/**
 *	���������Դ������
 */
void get_filetype(char *filename, char *filetype);

/**
 *	�ṩ��̬��Դ�������
 */
void serve_dynamic(int fd, char *filename, char *cgiargs);

/**
 *	��������
 */
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/**
 *  mserver ���(main)
 */
int main(int argc, char **argv)
{
	//-------------------------
    //	��������
    //-------------------------
	char *listen_port, client_port[MAXLINE], hostname[MAXLINE];
	int listenfd, connfd;
	struct sockaddr_storage clientaddr;	
	socklen_t clientlen;

	//-------------------------
    //	��������в���
    //-------------------------
	if (argc != 2)
	{
		//atoi();
		fprintf(stderr, "usage: %s <port> \n", argv[0]);
		exit(0);
	}
	
	listen_port = argv[1];
	//--------------------------------------------------------------------------
	//��ü��������� ��˼��Ϊʲôֻ��Ҫ��һ��������������������
	//��Ϊֻ��Ҫһ���������ɣ�һ���̸߳��������Ȼ�󴴽��µĽ��̻��߳���ִ������
	//--------------------------------------------------------------------------
	listenfd = Open_listenfd(listen_port);
	
	//���ϴ�����������
	while (1)
	{
		clientlen = sizeof(clientaddr);

		//��������׽���������
		connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		
		//��ÿͻ�����Ӧ��Ϣ(�������һ��Ϊflags = 0 ��ʾ����Ĭ����Ϊ)
		getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLINE, client_port, MAXLINE, 0);
		//�ͻ������ӳɹ�
		printf("Accepted connection from (%s, %s)\n", hostname, client_port);
		
		//����ͻ�������(����)
		doit(connfd);
		
		//�ر�����������
		close(connfd);
	}
	return 0;
}


/**
 *	��������(web����)
 */
void doit(int fd)
{
	//�ж��Ƿ��Ǿ�̬��Դ
	int is_static;
	//�ļ�������Ϣ
	struct stat sbuf;
	char filename[MAXLINE], cgiargs[MAXLINE];
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	my_rio_t rio;
	
	//--------------------
	//��ȡ�����к�����ͷ��
	//--------------------
	
	//��rio�������������󶨣�ֱ��ȥ����rio
	my_rio_readinitb(&rio, fd);
	//��ȡһ��---������
	my_rio_readlineb(&rio, buf, MAXLINE);
	printf("Request Lines:\n");
	printf("%s", buf);
	//ȡ�ò���
	sscanf(buf, "%s %s %s", method, uri, version);
	
	//��ʱ֧��GET
	if (strcasecmp(method, "GET"))
	{
		//������
		client_error(fd, method, "501", "Not implemented", "Mserver does not accetpt this method");
		return;
	}
		
	//��������ͷ��(ע���ڲ�ָ�����ƶ�)
	read_requesthdrs(&rio);
		
	//����uri���filename cgiargs ͬʱ�ж������uri�Ƿ�Ϸ�
	is_static = parse_uri(uri, filename, cgiargs);
	
	//�ж��ļ��Ƿ�����ҿɶ�
	if (stat(filename, &sbuf) < 0)
    { 
 		//˵���ļ�������
		client_error(fd, filename, "404", "Not found", "Sorry, Mserver couldn't find this file");
		return;
    }
	if (is_static) /* ��̬��Դ  */
	{
		//--------------------------------
        //	�����
   		//S_ISREG() - ����һ����ͨ�ļ���
		//S_ISDIR() - ����һ��Ŀ¼�ļ���
		//S_ISSOCK()- ����һ�������׽�����
		//--------------------------------
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode )) /* �ж��Ƿ�һ����ͨ�ļ�������Ȩ���� */
		{
			client_error(fd, filename, "403", "Forbidden", "Mserver couldn't read this file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else
	{	/* ��̬���� */
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) /* �ж��Ƿ���һ����ͨ�ļ��ҿ�ִ�� */
		{
			client_error(fd, filename, "403", "Forbidden", "Mserver couldn't run the CGI program");
			return;
		}
		serve_dynamic(fd, filename, cgiargs);
	}
	/*	
	while ((my_rio_readlineb(&rio, buf, MAXLINE)) != 0)
	{
		printf("%s\n", buf);
	}
	*/
	
	//��֪��Ϊʲô�����read��һֱ����
	//int res = read(fd, buf,2);
	//printf("res = %d  %s\n", res, buf);
		
	
	//send(fd, buf, strlen(buf), 1);
	//����һ���������ִ������������䣬��ͻ����޷�������ʾ����˵���Ϣ������ȷʵ�Ѿ��յ��ˣ����ǲ���ʾ
	//sleep(100);
}

/**
 * ��������ͷ�����򵥵���������ͷ����Ϣ��û�о��崦�����
 */
void read_requesthdrs(my_rio_t *rp)
{
	char buf[MAXLINE];
	
	my_rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n"))
	{
		//һ��һ�еĶ�ȡ(Ҳ���ȡ\n)
		my_rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);	
	}
	return;
}

/**
 *	����uri
 *	Ĭ�Ͻ���̬��Դ����cgi-binĿ¼��
 *	@return  1 : ��̬����   0 �� ��̬����  
 */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;

	//Ĭ������ҳ��index.html
	if (!strstr(uri, "cgi-bin")) /* ��̬��Դ  */
	{
		strcpy(cgiargs, ""); //��̬��Դ ���cgiargs
		strcpy(filename, "."); //�赱ǰ����Ŀ¼Ϊ��Ŀ¼
		strcat(filename, uri); //����uri�� /index.html ��filename���� ./index.html
	
		//����Ĭ�ϵ�Ŀ¼����ҳ����index.html
		if (uri[strlen(uri) - 1] == '/')
		{
			strcat(filename, "index.html");
		}
			return 1;
	} else { /* ��̬��Դ */
		//Ĭ�϶�̬��Դ�Ĺ���Ŀ¼��cgi-bin
		ptr = index(uri, '?'); //Ŀ����Ϊ�˻�ȡ����
		if (ptr)
		{
			strcpy(cgiargs, ptr + 1);
			//����������ptr
			*ptr = '\0';
		}	
		else
			//��������ֵ
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	
    }	

}

/**
 *	�ṩ��̬��Դ�������
 */
void serve_static(int fd, char *filename, int filesize)
{
	int srcfd; /* �ļ������� */
	char *srcp; /* �ļ�ӳ�䵽�ڴ��ָ�� */
	char filetype[MAXLINE], buf[MAXLINE];	

	//������Ӧͷ
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Mserver web Server\r\n", buf);
	sprintf(buf, "%sConnection: close\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	//�����Ϣ
	my_writen(fd, buf, strlen(buf));
	printf("Response headers:\n");
	printf("%s", buf);

	//��ͻ��˷�����Ϣ ���������ڴ�ӳ��
	srcfd = open(filename, O_RDONLY, 0); /* ֻ����ʽ�������ļ� */	
	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); /* ֱ�ӽ��ļ�ӳ�䵽�ڴ棬Ч�ʸ�(ϵͳ���ð���ҳ����ӳ���)*/
	close(srcfd);
	my_writen(fd, srcp, filesize);
	munmap(srcp, filesize); /* ����ڴ棬�����ڴ�й¶ */
	
}

/**
 *	���������Դ������
 *	ͨ���ļ����ƣ�����ļ�����
 */
void get_filetype(char *filename, char *filetype)
{
	//�ж��������Ƿ�����Ӵ�
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".png"))
		strcpy(filetype, "image/png");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else /* �޸�ʽ���� */
		strcpy(filetype,"text/plain");
}

/**
 *	�ṩ��̬��Դ�������
 *	����һ�����̣����ӽ�����ִ�г���
 */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
	char *emptylist[] = {NULL};
	char buf[MAXLINE];
	
	//����HTTP��Ӧ
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	my_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Mserver web server\r\n");
	my_writen(fd, buf, strlen(buf));

	if (fork() == 0) /* �����ӽ��� */
	{
		//���û�������
		setenv("QUERY_STRING", cgiargs, 1);
		dup2(fd, STDOUT_FILENO); /* �ض���(�����ִ�н�����������׼�����Ȼ���ض��������׽���) */
		execve(filename, emptylist, environ); /* ִ�г��� */
	}
	wait(NULL); /* ���ս��� */
}

/**
 *	��������
 * @fd ����������
 * @cause ��Դ����
 * @errnum ����״̬��
 * @shortmsg ������Ϣ
 * @longmsg ������ʾ��Ϣ	
 */
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXLINE];
	
	//������Ӧ����
	sprintf(body, "<html><title>Mserver Error</title>");
	sprintf(body, "%s<body bgcolor=\"ffffff\">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum,shortmsg);
	sprintf(body, "%s<p>%s:%s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Mserver web server</em>\r\n", body);

	//�����Ӧ��Ϣ
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	//������Ϣ
	my_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	my_writen(fd, buf, strlen(buf));
	//�����س�����
	sprintf(buf, "Content-length: %d\r\n\r\n",(int)strlen(body));
	//�����Ϣ
	my_writen(fd, buf, strlen(buf));
	//�����Ӧ����
	my_writen(fd, body, strlen(body));
}



#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* PROD_READ */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> //memcpy����
#include <sys/socket.h>
#include <netdb.h>
#include "rio.c"

#define LISTENQ 1024 //��ʾ���ں��ڿ�ʼ�ܾ���������֮ǰ��������Ҫ�Ŷӵ�δ��ɵ�������������
#define MAXLINE 8096
extern char **environ;
/********************************************************************
 *
 *
 *	socket��صĸ�������������ֱ��ʹ�ÿͻ��������ϣ����߷���˿�������
 *  ------------------- δ����main �������ִ��� ------------------- 
 *
 ********************************************************************/

//�����ͻ�������������
//int open_clientfd(char *hostname, char *port);

//����������һ������������(�����),����˼���ֻ��Ҫ����Ӧ�ļ����˿�
//int open_listenfd(char *port);

//�Զ����������
void my_error(char *mes)
{
	fprintf(stderr, "%s : %s\n", mes, strerror(errno));
	exit(0);
}


//�����д������accept()
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


//���������ؿͻ�������������(����ֱ���� unix i/o)
//socket() - connect() - ���clientfd
int open_clientfd(char *hostname, char *port)
{
	/*    ��������    */
	int clientfd;
	//hintsָ��һ��addrinfo�ṹ ���ṩ���׽��ֵ�ַ�б�ĸ��õĿ���
	struct addrinfo hints, *listp, *p;

	/*    ��ʼ�����ò���	*/
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM; //���óɷ������ӵ�addrinfo�ṹ	
	hints.ai_flags = AI_NUMERICSERV; //ʹ��(����)�˿�
	//�������
	hints.ai_flags |= AI_ADDRCONFIG;
	
	//������ӳɹ����׽��ֽṹ
	getaddrinfo(hostname, port, &hints, &listp);
	
	//����listpָ���addrinfo�ṹ ��������׽���������
	for (p = listp; p; p = p->ai_next)
	{
		//����socket������,���õ���getaddrinfo()����ʽ��Ŀ����Э���޹�
		if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
			//һֱ���Դ�����ֱ�������ɹ�
			continue;
		
		//�ͷ��������(clientfd�Ѿ������ɹ�),��ʱ�ͻ��˻�һֱ������connect() ���Ƿ��������ͨ�ɹ����߳������Ӵ���
		if ((connect(clientfd, p->ai_addr, p->ai_addrlen) != -1))
			//���ӳɹ�������ѭ��
			break;

		//����ʧ��
		close(clientfd); //�������� ��������clientfd
	}

	//�ͷ�listp
	freeaddrinfo(listp);
	//�ж�p��ǰ��ָ����׽��ֽṹ��Ϊ�����ʾһֱû�������ϣ�
	if (p)
		return clientfd;//���ӳɹ�
	else
		return -1; //����ʧ��
}

//����������һ������������(�����),����˼���ֻ��Ҫ����Ӧ�ļ����˿�
//socket() - bind() - listen()
int open_listenfd(char *port)
{
	/*    ��������    */
	struct addrinfo hints, *listp, *p;	
	int listenfd, optval = 1;

	/*    ����    */
	//��ʼ��hints
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM; //��������ѡ��
	//λ���룬���ÿ����޸�Ĭ����Ϊ����or(|) ��ϻ������
	hints.ai_flags = AI_PASSIVE; // �����־���߷��ص��׽��ֵ�ַ���ܱ����������� �����׽��� ͬʱ����
								 // �ں� �������������ܷ��͵�������������ip��ַ������
	hints.ai_flags |= AI_ADDRCONFIG; // ʹ�����ӵ�����
	hints.ai_flags |= AI_NUMERICSERV; //ǿ�Ʋ���service Ϊ�˿ں�
	
	//����׽��ֽṹ
	getaddrinfo(NULL, port, &hints, &listp);//���ڼ���
	
	//����
	for (p = listp; p; p = p->ai_next)
	{
		// ����һ������������
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			//ͬ��
			continue;
		
		//���÷�����ģʽ
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
		
		//�󶨼���������
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			//�󶨳ɹ�
			break; 
		close(listenfd); //�رգ����¼���
	}

	//�����ڴ�
	freeaddrinfo(listp);
	//�ڱ���listpʱ��û��һ���ɹ���(socket + binding)
	if (!p) return -1;

	//listen�������׽���ת��Ϊ һ�������׽��֣����Խ������Կͻ��˵���������
	//תΪ����������
	if (listen(listenfd, LISTENQ) < 0)
	{
		//�ر��ͷ�listentfd
		close(listenfd);
		return -1;
	}

	//���ؼ���������
	return listenfd;
}

/*    ��׳����غ���	*/
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

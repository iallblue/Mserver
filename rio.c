#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> //memcpy����
#define	MY_RIO_BUFSIZE 10240

typedef struct{
	int my_rio_fd;
	int my_rio_cnt;
	char *my_rio_bufptr;
	char my_rio_buf[MY_RIO_BUFSIZE];
}my_rio_t;


ssize_t my_readn(int fd, void * bufp, size_t n);
ssize_t my_writen(int fd, void * bufp, size_t n);

void my_rio_readinitb(my_rio_t *rp, int fd);
ssize_t my_rio_read(my_rio_t *rp, char *usrbuf, size_t n);
ssize_t my_rio_readlineb(my_rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t my_rio_readnb(my_rio_t *rp, void * usrbuf, size_t n);

/**
 *	main1()����
 */
int main1()
{
	int fd,nread,nwrite;
	char read_buf[100];
	char write_buf[100] = "hello";

	fd = open("foo.txt", O_CREAT|O_RDWR, 0666);
	nread = my_readn(fd, read_buf, 100);
	
	nwrite = my_writen(fd, write_buf, 100); //����һ��ͬʱ��д ����ִ���
	printf("nwrite = %d\n", nwrite);
	printf("nread = %d\n", nread);
	close(fd);
}

/**
 *
 *
 *	Ϊʲôʹ�����������������������Ϊ�˱���һֱ����read��writeϵͳ���ã�ͬʱ����� ����ֵ
 *
 */
ssize_t my_readn(int fd, void * bufp, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	//char *bufp;
	
	while (nleft > 0)
	{
		if ((nread = read(fd,(char *)bufp, n )) < 0)
		{
			if (errno == EINTR)
				nread = 0; //���¶�
			else 
				return -1; //��������
		}
		else if (nread == 0) //����EOF
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

ssize_t my_writen(int fd, void * bufp, size_t n)
{
	size_t nleft = n;
	ssize_t nwrite;//�����ķ���ֵ(ssize_t)

	//����Ȼ��ʣ��û��д�꣬�Ǿͼ������ļ���д
	while (nleft > 0)
	{
		if ((nwrite = write(fd, (char *)bufp, n)) <= 0)
		{
			if (errno == EINTR)
				nwrite = 0;
			else 
				return -1;
		}
		nleft -= nwrite;
		bufp += nwrite;
	}
	return n;
}

void my_rio_readinitb(my_rio_t *rp, int fd)
{
	rp->my_rio_fd = fd;//���ļ������� fd�� my_rio_fd��
	rp->my_rio_cnt = 0;
	rp->my_rio_bufptr = rp->my_rio_buf;
}


ssize_t my_rio_read(my_rio_t *rp, char *usrbuf, size_t n)
{
	int cnt;
	
	while (rp->my_rio_cnt <= 0) //��rp->my_rio_buf�ǿյ�ʱ��
	{
        	rp->my_rio_cnt = read(rp->my_rio_fd, rp->my_rio_buf, sizeof(rp->my_rio_buf));
		if (rp->my_rio_cnt < 0)
		{
			if (errno != EINTR) return -1;
		}
		else if (rp->my_rio_cnt == 0) //��ȡ��EOF 
			return 0;
		else 
			rp->my_rio_bufptr = rp->my_rio_buf; //����bufָ��
	}

	//ѡȡ��С n �� rio_cnt
	cnt = n;
	if (rp->my_rio_cnt < n)
		cnt = rp->my_rio_cnt;
	
	//���Ƶ��û��Ļ�����
	memcpy(usrbuf, rp->my_rio_bufptr, cnt);
	rp->my_rio_bufptr += cnt;
	rp->my_rio_cnt -= cnt;
	return cnt;
}

//�ȴ��ļ�rp�ж�ȡ�ڴ浽 my_rio_t�еĻ�������Ȼ����memcpy���û�ָ���Ļ�����
ssize_t my_rio_readlineb(my_rio_t *rp, void *usrbuf, size_t maxlen)
{
	int n,rc;
	char c, *bufp = usrbuf;

	for (n = 1; n < maxlen; n++)
	{
		if ((rc = my_rio_read(rp,&c,1))) //һ�ΰ�ȫ��ȡһ���ֽ�,��ŵ�usrbuf��
		{
			*bufp++ = c; //�Ὣ����Ҳ��ȡ��
			if (c == '\n')
			{
				n++;
				break;
			}
		}
		else if (rc == 0) 
		{
			if (n == 1)
				return 0; //EOF
			else
				break;
		}
		else
			return -1;
	}
	*bufp = '\0';
	return (n-1);//��ȡ���ַ�
}

//��my_readn����һ���汾
ssize_t my_rio_readnb(my_rio_t *rp, void * usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nread;
	char *bufp = usrbuf;
	
	while (nleft > 0)
	{
		if ((nread = my_rio_read(rp, bufp, nleft)) < 0)
			return -1;
		else if (nread == 0)
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}



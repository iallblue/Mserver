#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> //memcpy所需
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
 *	main1()测试
 */
int main1()
{
	int fd,nread,nwrite;
	char read_buf[100];
	char write_buf[100] = "hello";

	fd = open("foo.txt", O_CREAT|O_RDWR, 0666);
	nread = my_readn(fd, read_buf, 100);
	
	nwrite = my_writen(fd, write_buf, 100); //发现一旦同时读写 会出现错误
	printf("nwrite = %d\n", nwrite);
	printf("nread = %d\n", nread);
	close(fd);
}

/**
 *
 *
 *	为什么使用这样的输入输出函数，是为了避免一直调用read和write系统调用，同时处理好 不足值
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
				nread = 0; //重新读
			else 
				return -1; //遇到错误
		}
		else if (nread == 0) //遇到EOF
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

ssize_t my_writen(int fd, void * bufp, size_t n)
{
	size_t nleft = n;
	ssize_t nwrite;//函数的返回值(ssize_t)

	//当仍然有剩余没有写完，那就继续向文件中写
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
	rp->my_rio_fd = fd;//将文件描述符 fd和 my_rio_fd绑定
	rp->my_rio_cnt = 0;
	rp->my_rio_bufptr = rp->my_rio_buf;
}


ssize_t my_rio_read(my_rio_t *rp, char *usrbuf, size_t n)
{
	int cnt;
	
	while (rp->my_rio_cnt <= 0) //当rp->my_rio_buf是空的时候
	{
        	rp->my_rio_cnt = read(rp->my_rio_fd, rp->my_rio_buf, sizeof(rp->my_rio_buf));
		if (rp->my_rio_cnt < 0)
		{
			if (errno != EINTR) return -1;
		}
		else if (rp->my_rio_cnt == 0) //读取到EOF 
			return 0;
		else 
			rp->my_rio_bufptr = rp->my_rio_buf; //重置buf指针
	}

	//选取最小 n 和 rio_cnt
	cnt = n;
	if (rp->my_rio_cnt < n)
		cnt = rp->my_rio_cnt;
	
	//复制到用户的缓冲区
	memcpy(usrbuf, rp->my_rio_bufptr, cnt);
	rp->my_rio_bufptr += cnt;
	rp->my_rio_cnt -= cnt;
	return cnt;
}

//先从文件rp中读取内存到 my_rio_t中的缓冲区，然后在memcpy到用户指定的缓冲区
ssize_t my_rio_readlineb(my_rio_t *rp, void *usrbuf, size_t maxlen)
{
	int n,rc;
	char c, *bufp = usrbuf;

	for (n = 1; n < maxlen; n++)
	{
		if ((rc = my_rio_read(rp,&c,1))) //一次安全读取一个字节,存放到usrbuf中
		{
			*bufp++ = c; //会将换行也读取到
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
	return (n-1);//读取到字符
}

//是my_readn的另一个版本
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



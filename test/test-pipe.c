#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

int main()
{
	//声明管道数组
	//1：写端 0 ：读端
	int fd[2],n;
	pid_t pid;
	char line[1000], buf[100],*p;

	if (pipe(fd) < 0)
		printf("pipe error\n");
	
	if ((pid = fork()) < 0)
		printf("fork error\n");
	else if (pid > 0)
	{
		//父进程 - 写入数据
		printf("i come from father\n");
		p = "hello world\n";
		close(fd[0]);  /* 关闭读端 */
		write(fd[1],p, strlen(p));
		wait(pid);
	}
	else
	{
		//子进程
		close(fd[1]);  /* 关闭写端 */
		read(fd[0], line, 1000);	
		printf("this is from child:%s", line);
	}	
	exit(0); 
	return 0;
}

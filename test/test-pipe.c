#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

int main()
{
	//�����ܵ�����
	//1��д�� 0 ������
	int fd[2],n;
	pid_t pid;
	char line[1000], buf[100],*p;

	if (pipe(fd) < 0)
		printf("pipe error\n");
	
	if ((pid = fork()) < 0)
		printf("fork error\n");
	else if (pid > 0)
	{
		//������ - д������
		printf("i come from father\n");
		p = "hello world\n";
		close(fd[0]);  /* �رն��� */
		write(fd[1],p, strlen(p));
		wait(pid);
	}
	else
	{
		//�ӽ���
		close(fd[1]);  /* �ر�д�� */
		read(fd[0], line, 1000);	
		printf("this is from child:%s", line);
	}	
	exit(0); 
	return 0;
}

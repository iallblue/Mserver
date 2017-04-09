#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


int main()
{
	int sfd = dup(STDOUT_FILENO);
	int fd;
	
	printf("sfd = [%d]\n", sfd);
		
	/* 读写方式打开 + 追加信息 */
	fd = open("log.txt", O_RDWR | O_APPEND, 0);
	/* 重定向 */
	dup2(fd, STDOUT_FILENO);
	printf("this will show in the file stdoutfd = %d\n", STDOUT_FILENO);

	/* 重定向到标准输出 */
	dup2(sfd, STDOUT_FILENO);
	printf("this should be show stdout stdoutfd = %d\n", STDOUT_FILENO);
	close(fd);	
}

#include "socket_help.c"

int main()
{
	int fd;
	char buf[1024];
	printf("%p", environ);	
	fd = open("foo.txt", O_RDONLY, 0);
	int res = read(fd,buf,sizeof(buf));
	buf[res] = '\0';
	//必须手动添加\0
	printf("res=%d %s", res, buf);
	//printf("hello\n");
	return 0;
}

#include "socket_help.c"

int main()
{
	char *buf, content[1000], *p;
	char arg1[100], arg2[100];
	int i = 0,n1 = 0, n2 = 0;

	
	//获得环境变量参数
	/*
	while (envp[i])
	{
		printf("envp[%d] = %s\n", i++, envp[i]);
	}
	*/
	//int res = setenv("QUERY_STRING","1000&2000",1);	
	//printf("p1 = %p   p2 = %p\n", arg1, arg2);
	//return 1;	
	
	if ((buf = getenv("QUERY_STRING")) != NULL)
	{
		//printf("query_string is: %s\n", buf);
		//获得第一个变量的参数出现的位置
		p = strchr(buf, '&');	
		//将&替换为结束符
		*p = '\0';
		//return 0;
		strcpy(arg1, buf);
	
		strcpy(arg2, p+1);
				
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}
	else
	{
		printf("query_string is null\n");
	}

	//printf("n1 = %d, n2 = %d\n", n1, n2);
	
	//构造请求响应体(服务端做出的响应)
	//对于content是每次都追加
	sprintf(content,"QUERY_STRING=%s", buf);
	sprintf(content,"Welcome to my server");
	sprintf(content, "%sThe answer is :%d + %d = %d\r\n", content, n1, n2, n1+n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	//构造响应首部
	printf("Connection:close \r\n");
	printf("Content-length: %d\r\n", (int)strlen(content));	
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);
	
	return 0;
}

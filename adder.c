#include "socket_help.c"

int main()
{
	char *buf, content[1000], *p;
	char arg1[100], arg2[100];
	int i = 0,n1 = 0, n2 = 0;

	
	//��û�����������
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
		//��õ�һ�������Ĳ������ֵ�λ��
		p = strchr(buf, '&');	
		//��&�滻Ϊ������
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
	
	//����������Ӧ��(�������������Ӧ)
	//����content��ÿ�ζ�׷��
	sprintf(content,"QUERY_STRING=%s", buf);
	sprintf(content,"Welcome to my server");
	sprintf(content, "%sThe answer is :%d + %d = %d\r\n", content, n1, n2, n1+n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	//������Ӧ�ײ�
	printf("Connection:close \r\n");
	printf("Content-length: %d\r\n", (int)strlen(content));	
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);
	
	return 0;
}

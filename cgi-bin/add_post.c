#include "../socket_help.c"

int main()
{
	char *buf, content[1000], *p;
	char arg1[100], arg2[100];
	int i = 0,n1 = 0, n2 = 0;

	// post_buffer
	char post_buf[8000],c;
	int j = 0,len = 0;
	// setenv("CONTENT-LENGTH","100",1);
	// char *pp = getenv("CONTENT_LENGTH");
	if ((buf = getenv("CONTENT-LENGTH")) != NULL)
	{
		len = atoi(buf);
	}
	while (j < len)
	{
		c = fgetc(stdin);
		if (c == EOF) break;
		post_buf[j++] = c;
	}
	post_buf[j] = '\0';
	buf = post_buf;
	// strcpy(buf, post_buf, );
	if (strlen(buf) > 0)
	{
		//first
		char *p1 = strchr(buf, '=');
		p1++;
		p = strchr(buf, '&');
		*p = '\0';
		strcpy(arg1, p1);
		
		// second
		buf = p+1;
		char *p2 = strchr(buf, '=');
		strcpy(arg2, p2+1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}
	// scanf("%s", post_buf);/* 这种方法无法获取post数据 */
	//printf("%s\n", post_buf);
	// sprintf(content,"post_data=%s\r\n", post_buf);
	sprintf(content, "%sThe answer is :%d + %d = %d\r\n", content, n1, n2, n1+n2);
	sprintf(content,"%sWelcome to my post server", content);
	//sprintf(content, "%sThe answer is :%d + %d = %d\r\n", content, n1, n2, n1+n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	printf("Connection:close \r\n");
	printf("Content-length: %d\r\n", (int)strlen(content));	
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);
	


	
	return 0;
}//end of

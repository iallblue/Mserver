#include "../socket_help.c"

int main()
{
	char *buf, content[1000], *p;
	char arg1[100], arg2[100];
	int i = 0,n1 = 0, n2 = 0;

	
	//»ñµÃ»·¾³±äÁ¿²ÎÊý
	
/*	while (envp[i])
	{
		printf("envp[%d] = %s\n", i++, envp[i]);
	}*/
	
	//int res = setenv("QUERY_STRING","1000&2000",1);	
	//printf("p1 = %p   p2 = %p\n", arg1, arg2);
	//return 1;	
	
	/*	QUERY_STRING = a=123&b=234 */
	//int res = setenv("QUERY_STRING", "a=123&b=456",1);

	if ((buf = getenv("QUERY_STRING")) != NULL)
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
/*	if ((buf = getenv("QUERY_STRING")) != NULL)
	{
		//printf("query_string is: %s\n", buf);
		//»ñµÃµÚÒ»¸ö±äÁ¿µÄ²ÎÊý³öÏÖµÄÎ»ÖÃ
		p = strchr(buf, '&');	
		//½«&Ìæ»»Îª½áÊø·û
		*p = '\0';
		//return 0;
		strcpy(arg1, buf);
	
		strcpy(arg2, p+1);
				
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}*/
	else
	{
		printf("query_string is null\n");
	}

	//printf("n1 = %d, n2 = %d\n", n1, n2);
	
	//¹¹ÔìÇëÇóÏìÓ¦Ìå(·þÎñ¶Ë×ö³öµÄÏìÓ¦)
	//¶ÔÓÚcontentÊÇÃ¿´Î¶¼×·¼Ó
	sprintf(content,"QUERY_STRING=%s", buf);
	sprintf(content,"Welcome to my server");
	sprintf(content, "%sThe answer is :%d + %d = %d\r\n", content, n1, n2, n1+n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	//¹¹ÔìÏìÓ¦Ê×²¿
	printf("Connection:close \r\n");
	printf("Content-length: %d\r\n", (int)strlen(content));	
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);
	
	return 0;
}//end of
#include <stdlib.h>
#include <stdio.h>

int main()
{
	int a = 100;
	char *bufp,buf[100];

	sprintf(bufp, "%d", a);
	printf("%s\n", bufp);
}

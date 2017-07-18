#include <stdio.h>
void rever(char *s)
{
	char *h = s;
	char *t = s;
	
	while (*t++) {};
	t--;
	t--;
	while (h < t)
	{
		char ch = *t;
		*(t--) = *h;
		*(h++)= ch;
	}

}

void main()
{
	char *s = "hello";
	rever(s);
	printf("%s", s);
}


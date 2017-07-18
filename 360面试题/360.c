#include <stdio.h>
//1.实现strlen

int mystrlen(char*p)
{
	int length = 0;
	if (p == NULL)
		return 0;
	else
	{
		
		while (*p != '\0')
		{
			length++;
			p++;
		}
	}
	return length;
}

void main()
{
	char *p = "ssddas";
	int i = mystrlen(p);
	printf("%d", i);
	getchar();
}

//2.单链表逆置  递归实现

typedef struct Linknode
{
	int data;
	struct Linknode*pnext;
}node, *pnode;

pnode revgui(pnode p)
{
	if (p == NULL || p->pnext == NULL)
	{
		return p;
	}
	pnode next = p->pnext;
	revgui(next);
	p->pnext = p;
	p = NULL;
}


//3.字符串反转  char rever(char *s){填写实现}.hello->olleh

char* strrev(char* s)
{
	/* h指向s的头部 */
	char* h = s;
	char* t = s;
	char ch;

	/* t指向s的尾部 */
	while (*t++) {}
	t--;    /* 与t++抵消 */
	t--;    /* 回跳过结束符'\0' */

			/* 当h和t未重合时，交换它们所指向的字符 */
	while (h < t)
	{
		ch = *h;
		*h++ = *t;    /* h向尾部移动 */
		*t-- = ch;    /* t向头部移动 */
	}

	return s;
}

// 递归实现字符串反转     
char *reverse(char *str)
{
	if (!str)
	{
		return NULL;
	}

	int len = strlen(str);
	if (len > 1)
	{
		char ctemp = str[0];
		str[0] = str[len - 1];
		str[len - 1] = '/0';// 最后一个字符在下次递归时不再处理     
		reverse(str + 1); // 递归调用     
		str[len - 1] = ctemp;
	}

	return str;
}

//4.一个字符串中第一个只出现一次的字符  

char getone(char *p)
{
	if (p == NULL)
		return'\0';

	char ch = '\0';
	int len = strlen(p);
	for (int i = 0;i < len;i++)
	{
		int flag = 0;
		for (int j = i + 1;j < len;j++)
		{
			if (p[i] = p[j])
				flag = 1;
		}

		if (flag == 0)
		{
			ch = p[i];
			break;
		}
		else
		{
			ch = '\0';
		}

	}
}

	//哈希算法
char getonebyhash(char *p)
{
	int hashtable[256] = { 0 };//0-255 1字节 8位 8^2=256
	char ch;
	if (p == NULL)
		return'\0';
	char *phash = p;
	while (*phash != '\0')
	{
		hashtable[*(phash++)]++;//phash++ 指针移动 ++ 计数
							   //a[0] '\0'  a[49] '1' a[65] 'A'
	}
	phash = p;//恢复到开头
	while ((*phash) != '\0')
	{
		if ((hashtable[*phash] == 1))
		{
			ch = *phash;
			break;
		}
		phash++;
	}
	return ch;
}

//5.一个字符串按单词逆序

// 对指针p和q之间的所有字符逆序
void ReverseWord(char* p, char* q)
{
	while (p < q)
	{
		char t = *p;
		*p++ = *q;
		*q-- = t;
	}
}

// 将句子按单词逆序
char* ReverseSentence(char* s)
{
	// 这两个指针用来确定一个单词的首尾边界
	char* p = s; // 指向单词的首字符
	char* q = s; // 指向空格或者 '\0'

	while (*q != '\0')
	{
		if (*q == '')
		{
			ReverseWord(p, q - 1);
			q++; // 指向下一个单词首字符
			p = q;
		}
		else
			q++;
	}

	ReverseWord(p, q - 1); // 对最后一个单词逆序
	ReverseWord(s, q - 1); // 对整个句子逆序

	return s;
}
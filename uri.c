#include <stdio.h>
#include <string.h>
#include "uri.h"

int URLEncode(const char* src, int nSrcLen, char* des)
{
        char szBuf[4] = {0};
	int c = 0;
	int i;
	for (i = 0; i < nSrcLen; i++)
	{
		if ((src[i] >= 'A' && src[i] <= 'Z')
			|| (src[i] >= 'a' && src[i] <= 'z')
			|| (src[i] >= '0' && src[i] <= '9'))
		{
			des[c] = src[i];
			c++;
		}
		else if (src[i] == ' ')
		{
			des[c] = '+';
			c++;
		}
		else
		{
			sprintf(szBuf, "%%%02X", (unsigned char)src[i]);
			strcpy(des + c, szBuf);
			c+=3;
		}
	}
	des[c] = 0;
	return c;
}

char x2c(char* what)
{
	char digit;
	
	digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
	digit *= 16;
	digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));
	
	return digit;
}

int URLDecode(char* src)
{
	int x, y;
	for (x=0, y=0; src[y]; ++x, ++y)
	{
		if (src[y] == '+')
		{
			src[x] = ' ';
		}
		else if ((src[x] = src[y]) == '%')
		{
			src[x] = x2c(&src[y+1]);
			y+=2;
		}
	}
	return x;
}

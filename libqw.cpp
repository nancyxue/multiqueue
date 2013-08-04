#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serialize.h"

using namespace std;

int AddToLocalQueue(const char *queuefile, strmap &data, int cmd)
{
        SerializeResult Result;
	int datalen=serialize(data, Result);
	datalen++;      //包含'\0'
	int bSu = 0;
	int fd = open(queuefile, O_WRONLY | O_CREAT, 0644);
	if (-1 != fd)
	{
		if ( (-1 != flock(fd, LOCK_EX)) && (-1 != lseek(fd, 0, SEEK_END)) )
		{
			int bufflen = 4+16+datalen+4;    //[LEN4][HEADER16[ver4][cmd4][resv4][datalen4]][DATA][QEND]
			char *buff = (char *)malloc(bufflen);
			*(int *)buff = datalen+16;
			*(int *)(buff+4) = 0x00000001;
			*(int *)(buff+8) = cmd;
			*(int *)(buff+12) = 0;
			*(int *)(buff+16) = datalen;
			
			memcpy(buff+20, (const char *)Result, datalen);
			memcpy(buff+20+datalen, "QEND", 4);
			bSu = bufflen == write(fd, buff, bufflen);
			free(buff);
		}
		close(fd);
	}

	return bSu;
}


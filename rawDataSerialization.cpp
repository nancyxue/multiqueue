#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uri.h"

#define GETENV(name) getenv(name) ? getenv(name) : ""

#ifdef DEUBG_CGI_OUTPUT_FILE
#include <stdarg.h>
#include <time.h>
static const char* g_errorlog = "/tmp/cgi.log";
static int slog_write( const char* format, ... )
{
        char line[1024], *p = NULL;
	int size = 0;
	
	struct tm* newtime;
	time_t now;
	struct tm t;
	time(&now);
	newtime = localtime_r(&now, &t); // 支持多线程
	size = sprintf(line, "[%04d-%02d-%02d %02d:%02d:%02d] ",
					newtime->tm_year + 1900,
					newtime->tm_mon + 1,
					newtime->tm_mday,
					newtime->tm_hour,
					newtime->tm_min,
					newtime->tm_sec);
	p = line + size;
	
	va_list args;
	va_start(args, format);
	vsprintf(p, format, args);
	va_end(args);

	FILE* fp = fopen(g_errorlog, "a");
	if (fp)
	{
		fprintf(fp, "%s", line);
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "open file %s error\n", g_errorlog);
	}
}
#endif

char* GetPara(const char* src, const char* name, unsigned long* len)
{
	char* para = 0;
	char* pos = strstr(src, name);
	if (pos)
	{
		pos += strlen(name);
		char* pos2 = strchr(pos, '&');
		if (pos2)
		{
			para = (char*)malloc(pos2 - pos + sizeof(unsigned long) + 4);
			memcpy(para + sizeof(unsigned long), pos, pos2 - pos);
			para[pos2 - pos + sizeof(unsigned long)] = 0;
		}
		else
		{
			para = (char*)malloc(strlen(pos) + sizeof(unsigned long) + 4);
			strcpy(para+sizeof(unsigned long), pos);
		}
		*len = URLDecode(para+sizeof(unsigned long));
	}
	else
	{
		para = (char*)malloc(sizeof(unsigned long) + 4);
		*len = 0;
	}
	return para;
}

void ReleasePara(char* para)
{
	free(para);
}


#ifdef CGI_2_EXE
int doQuery(const char* config, const char* query)
#else
int doQuery(const char* query)
#endif
{
	int i;
	char szConf[128] = {0};
#ifdef CGI_2_EXE
	strcpy(szConf, config);
#else
	strcpy(szConf, GETENV("SCRIPT_FILENAME"));
	strcat(szConf, ".conf");
#endif
#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("config = %s\n", szConf);
#endif

	char szQFile[128] = {0};
	FILE* fp = fopen(szConf, "r");
	if (fp)
	{
		fgets(szQFile, sizeof(szQFile)-1, fp);
		for (i=strlen(szQFile); i>0; i--)
		{
			if (szQFile[i-1] == '\n' || szQFile[i-1] == '\r')
			{
				szQFile[i-1] = 0;
			}
			else
			{
				break;
			}
		}
		fclose(fp);
	}
#ifdef DEUBG_CGI_OUTPUT_FILE
	else
	{
		slog_write("open conf_file=%s error\n", szConf);
	}
#endif
#ifdef CGI_2_EXE
	else
	{
		printf("open conf_file=%s error\n", szConf);
	}
#endif

	unsigned int paralen;
	char* para = GetPara(query, "data=", &paralen);

	FILE* indexfp = fopen("./index.txt", "a+");
	fseek(indexfp, 0, SEEK_END);	
	char* ptr = "\n";
	unsigned int index = paralen + sizeof(unsigned long) + 4;
	fwrite(ptr, 1, 1, indexfp);                         // For line feed
	fwrite(&index, sizeof(index), 1, indexfp);	

	int bSu = 0;
	ssize_t bytes = 0;
	int fd = open(szQFile, O_WRONLY | O_CREAT, 0644);
	if (-1 != fd)
	{
		if ( (-1 != flock(fd, LOCK_EX)) && (-1 != lseek(fd, 0, SEEK_END)))
		{
			memcpy(para, &paralen, sizeof(paralen));
			memcpy(para + sizeof(paralen) + paralen, "QEND", 4);
			bytes = write(fd, para, sizeof(paralen) + paralen + 4);
			bSu = sizeof(paralen) + paralen + 4 == bytes;
		}
		#ifdef DEUBG_CGI_OUTPUT_FILE
		else
		{
			slog_write("lockf qdata_file=%s error\n", szQFile);
		}
		#endif
		#ifdef CGI_2_EXE
		else
		{
			printf("lockf qdata_file=%s error\n", szQFile);
		}
		#endif
		close(fd);
	}
#ifdef DEUBG_CGI_OUTPUT_FILE
	else
	{
		slog_write("open qdata_file=%s error\n", szQFile);
	}
	if (!bSu)
	{
		slog_write("error= %d:%s", errno, strerror(errno));
		slog_write("write data err: %d\n", bSu);
		slog_write("paralen=%u\n", paralen);
		slog_write("writen bytes=%d\n", bytes);
	}
#endif
#ifdef CGI_2_EXE
	else
	{
		printf("open qdata_file=%s error\n", szQFile);
	}
	if (!bSu)
	{
		printf("error= %d:%s", errno, strerror(errno));
		printf("write data err: %d\n", bSu);
		printf("paralen=%u\n", paralen);
		printf("writen bytes=%d\n", bytes);
	}
#endif
	ReleasePara(para);
	
	return bSu;
}

#ifdef CGI_2_EXE
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("USAGE:\n\t%s <conf> data=...\n", argv[0]);
		exit(1);
	}
	
	if (doQuery(argv[1], argv[2]))
	{
		printf("doQuery ok\n");
		return 0;
	}
	else
	{
		printf("doQuery fail\n");
		return 1;
	}
}
#else

int main()
{
#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("-----------\n");
#endif

	const char* method = GETENV("REQUEST_METHOD");

#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("method=%s\n", method);
#endif

	if (0 == strcmp("GET", method))
	{
		const char* querystring = GETENV("QUERY_STRING");
		
#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("querystring=%s\n", querystring);
#endif
		if (!doQuery(querystring))
		{
			return 0;
		}
#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("doQuery(get) ok\n");
#endif
	}
	else
	{
		int contentlength = atoi(GETENV("CONTENT_LENGTH"));

#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("contentlength=%d\n", contentlength);
#endif

		char* postdata = (char*)malloc(contentlength+1);
		memset(postdata, 0, contentlength+1);
		if (1 != fread(postdata, contentlength, 1, stdin))
		{

#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("fread post data from stdin error\n");
#endif
			free(postdata);
			return 0;
		}
		
#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("get postdata ok\n");
#endif
		if (!doQuery(postdata))
		{
			
#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("doQuery(post) return 0\n");
#endif
			free(postdata);
			return 0;
		}

#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("doQuery(post) ok\n");
#endif

		free(postdata);
	}

#ifdef DEUBG_CGI_OUTPUT_FILE
slog_write("before print header\n");
#endif
	
	printf("Content-Type: text/plain\r\n");
	printf("\r\nOK");

	return 0;
}
#endif

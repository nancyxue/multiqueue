#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include "arg.h"
#include "uri.h"

int CArg::PostArg(const char* host, int port, const char* query, const char* data, int len, int timeout)
{
        struct sockaddr_in sin;
	struct hostent*he = gethostbyname(host);
	if(!he)
	{
		//std::cout << "Err: gethostbyname("<< host << ")" << std::endl;
		return -1;
	}
	sin.sin_family = he->h_addrtype;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr =((unsigned long*)(he->h_addr_list[0]))[0];

	int sock;
	if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		//std::cout << "Err: socket init failed." << std::endl;
		return -1;
	}

	if(connect(sock,(struct sockaddr*)&sin,sizeof(sin)) < 0)
	{
		close(sock);
		//std::cout << "Err: socket connect failed." << std::endl;
		return -1;
	}
	
	char* szData = new char[len*3+1];
	int nDataLen = URLEncode(data, len, szData);
	char* szBuf = new char[1024 + 3 * len];
	sprintf(szBuf, "POST %s HTTP/1.0\r\nHost:%s\r\nContent-Length: %d\r\n\r\ndata=%s", query, host, nDataLen+5, szData);
	int ret = write(sock,szBuf,strlen(szBuf));
	delete[] szBuf;
	delete[] szData;
	if(ret < 0)
	{
		close(sock);
		//std::cout << "Err: socket write failed." << std::endl;
		return -1;
	}

	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);

	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	
	if(!select(sock + 1, &rfds, NULL, NULL, &tv))
	{
		close(sock);
		//std::cout << "Err: socket select failed." << std::endl;
		return -1;
	}

	if(!FD_ISSET(sock, &rfds))
	{
		close(sock);
		//std::cout << "Err: socket FD_ISSET failed." << std::endl;
		return -1;
	}

	int rv=0;
	szBuf = new char[1000+1];
	char* szTmp = szBuf;
	
	int iTmp = atoi(szTmp);                                                        
	int iBuf = atoi(szBuf);                                                
	
	while((rv = read(sock, szTmp, 1000 - iTmp + iBuf)))                
	{
		if (rv < 0)
		{
			delete[] szBuf;
			close(sock);
			std::cout << "Err: socket read failed." << std::endl;             
			return -1;
		}
		szTmp[rv] = 0;
		if ((rv + szTmp - szBuf) >= 12)
		{
			break;
		}
		szTmp += rv;
	}
	
	if (szBuf[9] != '2' || szBuf[10] != '0' || szBuf[11] != '0')
	{
		close(sock);
		std::cout << "Err: socket get http ret != 200." << std::endl;        
		delete[] szBuf;
		return -1;
	}
	
	delete[] szBuf;
	close(sock);
	return 0;
}


bool CArg::Post(const char* host, int port, const char* query, const char* data, int len, int timeout, string& response)   
{
	struct sockaddr_in sin;
	struct hostent* he = gethostbyname(host);
	if(!he)
	{
		std::cout << "Err: gethostbyname("<< host << ")" << std::endl;
		return false;
	}
	sin.sin_family = he->h_addrtype;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = ((unsigned long*)(he->h_addr_list[0]))[0];

	int sock;
	if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		std::cout << "Err: socket init failed." << std::endl;
		return false;
	}

	if(connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		close(sock);
		std::cout << "Err: socket connect failed." << std::endl;
		return false;
	}
	
	char* szBuf = new char[1024 + 3 * len];
	sprintf(szBuf, "POST %s HTTP/1.0\r\nContent-type: application/x-www-form-urlencoded\r\nHost:%s\r\nContent-Length: %d\r\n\r\n%s", query, host, len, data);
	
	std::cout << "Trace: " << szBuf << std::endl;
	
	int ret = write(sock,szBuf,strlen(szBuf));
	delete[] szBuf;
	if(ret < 0)
	{
		close(sock);
		std::cout << "Err: socket write failed." << std::endl;
		return false;
	}

	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);

	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	
	if(!select(sock + 1, &rfds, NULL, NULL, &tv))
	{
		close(sock);
		std::cout << "Err: socket select failed." << std::endl;
		return false;
	}

	if(!FD_ISSET(sock, &rfds))
	{
		close(sock);
		std::cout << "Err: socket FD_ISSET failed." << std::endl;
		return false;
	}

	int rv = 0;
	szBuf = new char[1000+1];
	char* szTmp = szBuf;
	
	int iTmp = atoi(szTmp);                                                     
	int iBuf = atoi(szBuf);                                                
	
	while((rv=read(sock, szTmp, 1000 - iTmp + iBuf)))                        
	{
		if (rv < 0)
		{
			delete[] szBuf;
			close(sock);
			std::cout << "Err: socket read failed." << std::endl;
			return false;
		}
		szTmp[rv] = 0;
		if ((rv + szTmp - szBuf) >= 1000)
		{
			break;
		}
		szTmp += rv;
	}
	
	if (szBuf[9] != '2' || szBuf[10] != '0' || szBuf[11] != '0')
	{
		close(sock);
		std::cout << "Err: socket get http ret != 200. response=" << szBuf << std::endl;
		delete[] szBuf;
		return false;
	}
	else
	{
		char* p = strstr(szBuf,"\r\n\r\n");
		if (!p) p = strstr(szBuf,"\n\n");
		
		if (p)
			response = (p+4);
		else
			response = "";
	}
	delete[] szBuf;
	close(sock);
	return true;
}

bool CArg::getUrlencodedStr(map<string,string>& data, string & result)
{
	result = "";
	map<string,string>::iterator iter;

	for (iter = data.begin(); iter != data.end(); iter++)
	{
		int len = iter->second.length();
		char* sData = new char[3*len+1];
		
		URLEncode(iter->second.c_str(), len, sData);
		
		if (result != "") result += "&";
		result += iter->first + "=" + sData;
		
		delete[] sData;
	}
	
	return true;
}

bool CArg::Post(const char* host, int port, const char* query, map<string,string>& data, int timeout, string& response)
{
	string request;
	
	if (CArg::getUrlencodedStr(data,request))
	{
		return CArg::Post(host,port,query,request.c_str(),request.length(),timeout,response);
	}
	else
	{
		std::cout << "Error: getUrlencodedStr" << std::endl;
		return false;
	}
}

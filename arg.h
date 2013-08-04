#ifndef ARG_H_
#define ARG_H_

#include <string>
#include <map>
using namespace std;


struct ARG_HEAD
{
        unsigned int uVersion;                                        
	unsigned int uCmd;                                            
	unsigned int uCount;                                          
	unsigned int uLen;                                                                              
};

class CArg
{
public:
	static int PostArg(const char* host, int port, const char* query, const char* data, int len, int timeout);  
	static bool Post(const char* host, int port, const char* query, const char* data, int len, int timeout, string& response);   
	static bool Post(const char* host, int port, const char* query, map<string,string>& data, int timeout, string& response);  
private:
	static bool getUrlencodedStr(map<string,string>& data, string & result);
};

#endif //ARG_H_

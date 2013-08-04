#ifndef COMMONFUNC_H_
#define COMMONFUNC_H_

#include <string>
#include <vector>
#include <map>

class CCommonFunc
{
public:
        static int isDir(const char * dir);
	static int isReg(const char * file);
	static int isLnk(const char * link);
	
	static void splitString(const char * src, char c, std::vector<std::string> & slist, int limit);
	static void str2map(const char * strRecord, std::map<std::string, std::string> & mapRecord);
	static std::string map2str(const std::map<std::string, std::string> & mapRecord);
	
	static std::string encode(const char * src);
	static std::string decode(const char * src);
	static void mapdecode(std::map<std::string, std::string> & mapRecord);
	
	static std::string trim(const char * src);
};

#endif //COMMONFUNC_H_

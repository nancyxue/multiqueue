#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "commonfunc.h"

//require:
//  dir != NULL
//return:
//	0 -- 不是目录
//	1 -- 是目录
int CCommonFunc::isDir(const char* dir)
{
	struct stat st;
	if (0 == stat(dir, &st))
	{
		return S_ISDIR(st.st_mode);
	}
	return 0;
}

//require:
//	file != NULL
//return:
//	0 -- 不是普通文件
//	1 -- 是普通文件
int CCommonFunc::isReg(const char* file)
{
	struct stat st;
	if (0 == stat(file, &st))
	{
		return S_ISREG(st.st_mode);
	}
	return 0;
}

//require:
//	link != NULL
//return:
//	0 -- 不是符号链接
//	1 -- 是符号链接
int CCommonFunc::isLnk(const char* link)
{
	struct stat st;
	if (0 == stat(link, &st))
	{
		return S_ISLNK(st.st_mode);
	}
	return 0;
}

void CCommonFunc::splitString(const char* src, char c, std::vector<std::string>& slist, int limit)
{
	int count = 1;
	std::string str;
	for (int i = 0; src[i]; ++i)
	{
		if (src[i] == c && (limit == 0 || count < limit))
		{
			count ++;
			slist.push_back(str);
			str = "";
		}
		else
		{
			str += src[i];
		}
	}
	slist.push_back(str);
}

void CCommonFunc::str2map(const char* strRecord, std::map<std::string, std::string>& mapRecord)
{
	mapRecord.clear();
	
	std::vector<std::string> slist;
	CCommonFunc::splitString(strRecord, '\n', slist, 0);
	
	for (std::vector<std::string>::const_iterator i = slist.begin(); i!=slist.end(); ++i)
	{
		std::vector<std::string> pair;
		CCommonFunc::splitString(i->c_str(), ':', pair, 2);
		if (pair.size() == 2)
		{
			mapRecord[pair[0]] = pair[1];
		}
	}
}

std::string CCommonFunc::map2str(const std::map<std::string, std::string>& mapRecord)
{
	std::string strRecord;
	std::map<std::string, std::string>::const_iterator i;
	for (i = mapRecord.begin(); i != mapRecord.end(); ++i)
	{
		strRecord += i->first + ":" + CCommonFunc::encode(i->second.c_str()) + "\n";
	}
	return strRecord;
}

std::string CCommonFunc::encode(const char* src)
{
	std::string des;
	for (int i = 0; src[i]; ++i)
	{
		if (src[i] == '\\')
		{
			des += "\\\\";
		}
		else if (src[i] == '\n')
		{
			des += "\\n";
		}
		else if (src[i] == '\r')
		{
			des += "\\r";
		}
		else
		{
			des += src[i];
		}
	}
	return des;
}

std::string CCommonFunc::decode(const char* src)
{
	std::string des;
	for (int i = 0; src[i]; ++i)
	{
		if (src[i] == '\\')
		{
			if (src[i+1] == '\\')
			{
				des += "\\";
				++i;
			}
			else if (src[i+1] == 'n')
			{
				des += "\n";
				++i;
			}
			else if (src[i+1] == 'r')
			{
				des += "\r";
				++i;
			}
			else
			{
				des += src[i];
			}
		}
		else
		{
			des += src[i];
		}
	}
	return des;
}

void CCommonFunc::mapdecode(std::map<std::string, std::string>& mapRecord)
{
	std::map<std::string, std::string>::iterator i;
	for (i = mapRecord.begin(); i != mapRecord.end(); ++i)
	{
		i->second = CCommonFunc::decode(i->second.c_str());
	}
}

std::string CCommonFunc::trim(const char* src)
{
	std::string des;
	int start = 0;
	int end = strlen(src);
	for ( ; start<end; ++start)
	{
		if (src[start] != ' ' && src[start] != '\t' && src[start] != '\n' && src[start] != '\r')
		{
			break;
		}
	}
	for ( ; end>start; --end)
	{
		if (src[end-1] != ' ' && src[end-1] != '\t' && src[end-1] != '\n' && src[end-1] != '\r')
		{
			break;
		}
	}
	for (int i = start; i < end; ++i)
	{
		des += src[i];
	}
	return des;
}

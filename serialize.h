#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <string>
#include <map>

using namespace std;

typedef map<string, string> strmap;

class SerializeResult
{
private:
        char* m_result;
public:
	SerializeResult() { m_result = NULL; }
	~SerializeResult()
	{
		if (m_result)
			delete [] m_result;
	}
	char* GetBuff(int size)
	{
		if (m_result)
			delete [] m_result;
		return m_result = new char [size];
	}
	operator const char* (){return m_result;}
	const char* operator *(){return m_result;}
};

int unserialize(istream& data, strmap& datamap);
int unserialize(const char* data, int size, strmap& datamap);
int serialize(const strmap& datamap, SerializeResult& Result);

#endif

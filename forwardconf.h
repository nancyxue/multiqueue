#ifndef FORWARDCONF_H_
#define FORWARDCONF_H_

#include "luaop.h"

class CForwardConf
{
public:
        std::string m_host;
	unsigned long m_port;
	std::string m_query;
	unsigned long m_id;

	void load(lua_State * L);
	void dump();
	int forward(const char * data, const char * newData);
};

#endif //FORWARDCONF_H_

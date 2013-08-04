#include <unistd.h>
#include <iostream>
#include "arg.h"
#include "forwardconf.h"

void CForwardConf::load(lua_State * L)
{
        m_host = CLuaOp::lua_getStringItem(L, "host");
	m_port = CLuaOp::lua_getIntItem(L, "port");
	m_query = CLuaOp::lua_getStringItem(L, "query");
	m_id = CLuaOp::lua_getIntItem(L, "id");
}

void CForwardConf::dump()
{
	std::cout << "\thost: " << m_host << std::endl;
	std::cout << "\tport: " << m_port << std::endl;
	std::cout << "\tquery: " << m_query << std::endl;
	std::cout << "\tid: " << m_id << std::endl;
}

int CForwardConf::forward(const char * data, const char * newData)
{
	int len = strlen(newData);
	
	char * fData = new char[sizeof(ARG_HEAD) + len];
	memcpy(fData, data, sizeof(ARG_HEAD));
	ARG_HEAD * perrHead = (ARG_HEAD *)fData;
	perrHead->uCmd = m_id;
	perrHead->uLen = len;
	memcpy(fData+sizeof(ARG_HEAD), newData, len);
	
	int ret;
	int retry;
	for (retry = 0; retry < 5; ++retry)
	{
		ret = CArg::PostArg(m_host.c_str(), m_port, m_query.c_str(),
			fData, sizeof(ARG_HEAD) + len, 3);
		if (0 == ret)
		{
			break;
		}
		sleep(retry);
	}
	
	delete[] fData;
	
	return ret;
}

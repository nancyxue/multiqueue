#include <iostream>
#include "luaop.h"
#include "arg.h"
#include "mysqlop.h"

CMysqlOp::CMysqlOp()
{
        m_mysql = NULL;
}

CMysqlOp::~CMysqlOp()
{
	if (m_mysql)
	{
		mysql_close(m_mysql);
	}
}

void CMysqlOp::load(const char* filename)
{
	lua_State* L = luaL_newstate();//lua_open();
	
	luaL_openlibs(L);
	
	luaL_dofile(L, filename);
	
	lua_getglobal(L, "mysqlop");

	if (lua_istable(L, -1))
	{
		m_host = CLuaOp::lua_getStringItem(L, "host");
		m_port = CLuaOp::lua_getIntItem(L, "port");
		m_database = CLuaOp::lua_getStringItem(L, "database");
		m_user = CLuaOp::lua_getStringItem(L, "user");
		m_pass = CLuaOp::lua_getStringItem(L, "pass");
		m_unix_socket = CLuaOp::lua_getStringItem(L, "unix_socket");
	}

	lua_close(L);

	mysql_init(&m_realmysql);
	m_mysql = mysql_real_connect(&m_realmysql,
		m_host.c_str(),
		m_user.c_str(),
		m_pass.c_str(),
		m_database.c_str(),
		m_port,
		m_unix_socket.length() ? m_unix_socket.c_str() : NULL, 0);
}

void CMysqlOp::dump()
{
	std::cout << "mysqlop:" << std::endl;
	std::cout << "\thost: " << m_host << std::endl;
	std::cout << "\tport: " << m_port << std::endl;
	std::cout << "\tdatabase: " << m_database << std::endl;
	std::cout << "\tuser: " << m_user << std::endl;
	std::cout << "\tpass: " << m_pass << std::endl;
	std::cout << "\tunix_socket: " << m_unix_socket << std::endl;
	std::cout << "\tConnected: " << (m_mysql ? "true" : "false") << std::endl;
	std::cout << std::endl;
}

int CMysqlOp::update(const char* sql)
{
	std::cout << "SQL: " << sql << std::endl;
	if (!m_mysql)
	{
		std::cout << "ERROR: NULL" << std::endl;
		return -1;
	}
	
	int ret = mysql_real_query(m_mysql, sql, strlen(sql));
	if (ret)
	{
		std::cout << "ERROR: " << mysql_errno(m_mysql) << " " << mysql_error(m_mysql) << std::endl;
	}
	return ret;
}

int CMysqlOp::select(const char* sql, std::vector<std::map<std::string, std::string> > & result)
{
	std::cout << "SQL: " << sql << std::endl;
	result.clear();
	if (!m_mysql)
	{
		std::cout << "ERROR: NULL" << std::endl;
		return -1;
	}
	
	int ret = mysql_real_query(m_mysql, sql, strlen(sql));
	if (ret)
	{
		std::cout << "ERROR: " << mysql_errno(m_mysql) << " " << mysql_error(m_mysql) << std::endl;
		return ret;
	}

	MYSQL_RES* res;
	MYSQL_ROW row;
	MYSQL_FIELD* fields;
	int colnum;
	
	if (!(res = mysql_store_result(m_mysql)))
	{
		std::cout << "ERROR: " << mysql_errno(m_mysql) << " " << mysql_error(m_mysql) << std::endl;
		return -2;
	}

	colnum = mysql_num_fields(res);
	while (row=mysql_fetch_row(res))
	{
		std::map<std::string, std::string> item;
		for (int i=0; i<colnum; ++i)
		{
			fields = mysql_fetch_field_direct(res, i);
			item[fields->name] = row[i];
		}
		result.push_back(item);
	}
	mysql_free_result(res);
	
	return 0;
}

int CMysqlOp::insert_id()
{
	if (m_mysql)
	{
		return mysql_insert_id(m_mysql);
	}
	return 0;
}

int CMysqlOp::affected_rows()
{
	if (m_mysql)
	{
		return mysql_affected_rows(m_mysql);
	}
	return 0;
}

std::string CMysqlOp::escape_string(const char* from)
{
	if (!from)
	{
		return "";
	}
	int len = strlen(from);
	char* to = new char[len * 2 + 1];
	mysql_escape_string(to, from, len);
	std::string ret = to;
	delete[] to;
	return ret;
}

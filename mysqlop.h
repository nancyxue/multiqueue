#ifndef MYSQLOP_H_
#define MYSQLOP_H_

#include <map>
#include <vector>
#include <string>
#include <./mysql/mysql.h>

class CMysqlOp
{
public:
        CMysqlOp();
	virtual ~CMysqlOp();
	
	void load(const char* filename);
	void dump();

	int update(const char* sql);
	
	int select(const char* sql, std::vector<std::map<std::string, std::string> >& result);
	
	int insert_id();
	
	int affected_rows();
	
	static std::string escape_string(const char* from);
	
	int err(const char* data, const char* newData);

protected:
	MYSQL m_realmysql;
	MYSQL* m_mysql;

	std::string m_host;
	unsigned long m_port;
	std::string m_database;
	std::string m_user;
	std::string m_pass;
	std::string m_unix_socket;
};

#endif //MYSQLOP_H_

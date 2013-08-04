#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <string>
#include <map>
#include <unistd.h>
#include <signal.h>
 
using namespace std;


typedef int (*DATAFUNC)(const char *, unsigned int);
typedef int (*BATCHFUNC)(const char *, unsigned int, unsigned int);
typedef int (*SERIALFUNC)(const char *, unsigned int, map<string, string> &);
typedef map<string, DATAFUNC> _COMMANDS;
typedef map<string, BATCHFUNC> _BATCHCMDS;
typedef map<string, SERIALFUNC> _SERIALCMDS;

void _log(char* message);

class PIPE
{
public:
        PIPE();
	virtual ~PIPE();

	void load(const char * path);
	int exec(const char * data, unsigned int len);
	int execBatch(const char * data, unsigned int len, unsigned int cmd);
	//int read_exec();
	
	void clearfd(int & fd) { if (fd) { close(fd); fd = 0; } }
	void clearpid(pid_t & pid) { if (pid) { kill(pid, SIGTERM); pid = 0;} }
	
	pid_t m_pid;
	int m_fd1[2];
	int m_fd2[2];
};

typedef map<string, PIPE> _PIPE;

class LIBRARY
{
public:
	LIBRARY();
	virtual ~LIBRARY();
	
	void * m_handle;
	_COMMANDS m_funcs;
	_BATCHCMDS m_batchs;
	_SERIALCMDS m_serials;
	
	void load(const char * module, const char * func);
	int exec(const char * func, const char * data, unsigned int len);

	void loadBatch(const char * module, const char * func);
	int execBatch(const char * func, const char * data, unsigned int len, unsigned int cmd);

	void loadSerial(const char * module, const char * func);
	int execSerial(const char * func, const char * data, unsigned int len, map<string, string> & mapItem);

};

typedef map<string, LIBRARY> _LIBRARYS;

////////////////////////////////////////////////////////////////////////////

class COMMANDS
{
public:
	static _COMMANDS m_commands;
	static _LIBRARYS m_librarys;
	static _PIPE m_pipes;
	COMMANDS();
};

extern COMMANDS commands;

#endif //HANDLE_H_



#include <iostream>
#include <dlfcn.h>
#include "handle.h"
#include "arg.h"

void _log(char* message)
{
        FILE* fp = fopen("log.txt", "ab");
	fprintf(fp, "%s\n", message);
	fclose(fp);
}

PIPE::PIPE()
{
	m_fd1[0] = 0;
	m_fd1[1] = 0;
	m_fd2[0] = 0;
	m_fd2[1] = 0;
	m_pid = 0;
}

PIPE::~PIPE()
{
	clearpid(m_pid);
	clearfd(m_fd1[0]);
	clearfd(m_fd1[1]);
	clearfd(m_fd2[0]);
	clearfd(m_fd2[1]);
}

void PIPE::load(const char * path)
{	
	if (m_pid)
	{
		return;
	}
	
	if (pipe(m_fd1) < 0 || pipe(m_fd2) < 0)
	{
		return;
	}

	if ((m_pid = fork()) < 0)
	{
		m_pid = 0;
		return;
	}
	else if (m_pid > 0)	//parent
	{
		clearfd(m_fd1[0]);
		clearfd(m_fd2[1]);
	}
	else			//child
	{
		clearfd(m_fd1[1]);
		clearfd(m_fd2[0]);
		
		if (m_fd1[0] != STDIN_FILENO)
		{
			if (dup2(m_fd1[0], STDIN_FILENO) != STDIN_FILENO)
			{
				clearfd(m_fd1[0]);
			}
		}

		if (m_fd2[1] != STDOUT_FILENO)
		{
			if (dup2(m_fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				clearfd(m_fd2[1]);
			}
		}

		execl(path, 0);
		exit(0);
	}
}

int PIPE::exec(const char * data, unsigned int len)
{
	if (m_pid == 0)
	{
		return -1;
	}
	
        if (sizeof(unsigned int) != write(m_fd1[1], &len, sizeof(unsigned int)))
	{
		return -1;
	}

	if ((ssize_t)len != write(m_fd1[1], data, len))
	{
		return -1;
	}

	char c;
	if(1 != read(m_fd2[0], &c, 1))
	{
		return -1;
	}

	if(c)
	{
		return -1;
	}

	return 0;
}

/*
int PIPE::read_exec()
{
	if (m_pid == 0)
        {
                return -1;
        }
        
        char c;
	fd_set fd_pipe_set; //声明fd_set 类型的 存储要检测的pipe句柄
	struct timeval tv;	// select 等待的时间 微秒单位
	int ret;	// 返回值
	FD_ZERO( &fd_pipe_set );	// 初始化
	FD_SET( m_fd2[0], &fd_pipe_set ); // // 把要检测的句柄pipe加入到集合里 
	tv.tv_sec = 0;
	tv.tv_usec = 3; // 30微秒
	ret = select( m_fd2[0] + 1, &fd_pipe_set, NULL, NULL, &tv );// 检测我们上面设置到集合里的句柄是否有可读信息 
	if( ret < 0 )
	{
		//std::cout << "|--flush_idle read_exec() print : Select ERROR ! \n" << std::endl;
            	return -1;		// select error
        }
	else if( ret == 0 )	// 超时了，pipe里没有变化。
	{
		//std::cout << "|--flush_idle read_exec() print : Select TIMEOUT ! \n" << std::endl;
		return -2;
        }
	else
	{
		if(FD_ISSET(m_fd2[0], &fd_pipe_set)) // 句柄是否真的变成可读的了 
		{
			if (1 != read(m_fd2[0], &c, 1)) // 对于读而言，非阻塞就是立即返回
        		{
				return -3;
        		}
        		if (c)
        		{
				return -4;
			}
		}
	}
	return 0;
}
*/

int PIPE::execBatch(const char * data, unsigned int len, unsigned int cmd)
{
	if (m_pid == 0)
	{
		return -1;
	}
	
	if (sizeof(unsigned int) != write(m_fd1[1], &cmd, sizeof(unsigned int)))
	{
		return -1;
	}
	
	if (sizeof(unsigned int) != write(m_fd1[1], &len, sizeof(unsigned int)))
	{
		return -1;
	}
	
	if ((ssize_t)len != write(m_fd1[1], data, len))
	{
		return -1;
	}
	
	char c;
	if (1 != read(m_fd2[0], &c, 1))
	{
		return -1;
	}

	if (c)
	{
		return -1;
	}
	
	return 0;
}

LIBRARY::LIBRARY()
{
	m_handle = NULL;
}

LIBRARY::~LIBRARY()
{
	if (m_handle)
	{
		dlclose(m_handle);
	}
}

void LIBRARY::load(const char * module, const char * func)
{
	if (!m_handle)
	{
		m_handle = dlopen(module, RTLD_LAZY);
		if (!m_handle)
		{
			std::cout << dlerror() << std::endl;
		}
	}
	
	if (m_handle)
	{
		_COMMANDS::const_iterator it = m_funcs.find(func);
		if (it == m_funcs.end())
		{
			m_funcs[func] = (DATAFUNC)dlsym(m_handle, func);
			if (!m_funcs[func])
			{
				std::cout << dlerror() << std::endl;
			}
		}
	}
}

int LIBRARY::exec(const char * func, const char * data, unsigned int len)
{
	_COMMANDS::iterator it = m_funcs.find(func);
	if (it != m_funcs.end() && it->second)
	{
		return (0 == (*(it->second))(data, len)) ? 0 : -1;
	}
	return -1;
}

void LIBRARY::loadBatch(const char * module, const char * func)
{
	if (!m_handle)
	{
		m_handle = dlopen(module, RTLD_LAZY);
		if (!m_handle)
		{
			std::cout << dlerror() << std::endl;
		}
	}
	
	if (m_handle)
	{
		_BATCHCMDS::const_iterator it = m_batchs.find(func);
		if (it == m_batchs.end())
		{
			m_batchs[func] = (BATCHFUNC)dlsym(m_handle, func);
			if (!m_batchs[func])
			{
				std::cout << dlerror() << std::endl;
			}
		}
	}
}

int LIBRARY::execBatch(const char * func, const char * data, unsigned int len, unsigned int cmd)
{
	_BATCHCMDS::iterator it = m_batchs.find(func);
	if (it != m_batchs.end() && it->second)
	{
		return (0 == (*(it->second))(data, len, cmd)) ? 0 : -1;
	}
	return -1;
}

void LIBRARY::loadSerial(const char * module, const char * func)
{
	if (!m_handle)
	{
		m_handle = dlopen(module, RTLD_LAZY);
		if (!m_handle)
		{
			std::cout << dlerror() << std::endl;
		}
	}
	
	if (m_handle)
	{
		_SERIALCMDS::const_iterator it = m_serials.find(func);
		if (it == m_serials.end())
		{
			m_serials[func] = (SERIALFUNC)dlsym(m_handle, func);
			if (!m_serials[func])
			{
				std::cout << dlerror() << std::endl;
			}
		}
	}
}

int LIBRARY::execSerial(const char * func, const char * data, unsigned int len, std::map<std::string, std::string> & mapItem)
{
	_SERIALCMDS::iterator it = m_serials.find(func);
	if (it != m_serials.end() && it->second)
	{
		return (0 == (*(it->second))(data, len, mapItem)) ? 0 : -1;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////

_COMMANDS COMMANDS::m_commands;
_LIBRARYS COMMANDS::m_librarys;                                                                                         
_PIPE COMMANDS::m_pipes;
COMMANDS commands;                                                        

int outputX(const char * data, unsigned int len)
{
	std::cout << "------------------- outputX -------------------" << std::endl;
	for (unsigned int i=0; i<len; ++i)
	{
		if (i % 16 == 0)
		{
			if (i != 0 && i!= len-1)
			{
				std::cout << std::endl;
			}
		}
		printf("%02X ", (unsigned char)data[i]);
	}
	std::cout << "\n------------------- outputX -------------------" << std::endl;
	return 0;
}

int outputS(const char * data, unsigned int len)
{
	std::cout << "------------------- outputS -------------------" << std::endl;
	for (unsigned int i=16; i<len; ++i)
	{
		printf("%c", data[i]);
	}
	std::cout << "\n------------------- outputS -------------------" << std::endl;
	return 0;
}

COMMANDS::COMMANDS()
{
	m_commands["outputX"] = outputX;
	m_commands["outputS"] = outputS;
}


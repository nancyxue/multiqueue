#ifndef  CPP_LOG_H_
#define  CPP_LOG_H_
#include <string>
#include <vector>
#include <iostream>
#include <fstream> 
#include <stdarg.h>
#include "pthread.h"
using namespace std;


class CPPLog
{
public:
        ///////////////////////////////////////////////
	//	函数：CPPLog
	//	说明：缺省构造函数，将输出打印到标准输出
	///////////////////////////////////////////////
	CPPLog();

	///////////////////////////////////////////////
	//	函数：CPPLog
	//	功能：构造函数，将输出打印到文件
	//	参数：	logName - 文件名
	//			autoClear-是否文件超过一定长度自动清空
	//			maxSize - 日志文件最大尺寸
	///////////////////////////////////////////////
	CPPLog( const char* logName, bool autoClear = false, int m_maxLogSize = 0 );
	
	~CPPLog();

	///////////////////////////////////////////////
	//	函数：<<
	//	说明：模板函数，重载了输出操作符
	///////////////////////////////////////////////
	CPPLog& operator << (int);
	CPPLog& operator << (unsigned long);
	CPPLog& operator << (const char*);
	CPPLog& operator << (string& );

//	template<typename T>
//	CPPLog& operator << (const T&);
//
//	template<typename T>
//	CPPLog& operator << (const T*);


	///////////////////////////////////////////////
	//	函数：<<
	//	说明：重载了输出操作符，支持endl等控制符
	///////////////////////////////////////////////
	CPPLog& operator << ( CPPLog& (*op)(CPPLog&) );

	///////////////////////////////////////////////
	//	函数：print
	//	说明：打印格式化内容到设备
	///////////////////////////////////////////////
	bool print(const char *fmt, ...);

	///////////////////////////////////////////////
	//	函数：print_nts 
	//	说明：不抢锁的打印格式化内容到设备，不线程安全
	///////////////////////////////////////////////
	bool print_nts(const char *fmt, ...);

	///////////////////////////////////////////////
	//	函数：lock/unlock 
	//	说明：加锁/去锁
	///////////////////////////////////////////////
	void lock();
	void unlock();

	///////////////////////////////////////////////
	//	函数：endl/flush
	//	说明：两个友元函数，让CPPLog支持endl等控制符
	///////////////////////////////////////////////
	friend CPPLog& endl(CPPLog& log);  
	friend CPPLog& flush(CPPLog& log);   

private:
	///////////////////////////////////////////////
	//	函数：flush
	//	说明：将输出缓冲内容刷新到设备
	///////////////////////////////////////////////
	CPPLog& flush();


private:
	bool		m_logToFile;	//ture-将日志输出到文件;false-输出到标准输出
	string		m_logFilename;	//日志文件名
	ofstream	m_logStream;	//日志文件流
	bool		m_autoClear;	//是否日志文件超过一定尺寸自动清理
	int	        m_maxLogSize;	//日志文件最大尺寸；当m_autoClear==true时有效
	pthread_mutex_t m_fileLock;	//日志文件的锁
};

#endif

#include "CPPLog.h"
#include "util.h"
using namespace std;

CPPLog::CPPLog() : m_logToFile(false)
{
        if(pthread_mutex_init(&m_fileLock, NULL) != 0)
	{
		cerr << "Error: Failed to init the file mutex lock." << endl;
		exit(1);
	}
}


CPPLog::CPPLog( const char* logName, bool autoClear, int m_maxLogSize ):
	m_logToFile(true), m_logFilename(logName),
	m_logStream(logName, ofstream::app),
	m_autoClear(autoClear), m_maxLogSize(m_maxLogSize)
{
	if (pthread_mutex_init(&m_fileLock, NULL) != 0)
	{
		cerr << "Error: Failed to init the file mutex lock." << endl;
		exit(1);
	}
	if (!m_logStream)
	{
		cerr << "Error: Failed to open the file stream for file:" << logName << endl;
		exit(1);
	}
}


CPPLog::~CPPLog()
{
	if (m_logToFile)
	{
		m_logStream.close();
	}
	pthread_mutex_destroy(&m_fileLock);
}


CPPLog& CPPLog::operator << (int obj)
{
	CMutexLock lock(m_fileLock);
	
	if (m_logToFile)
	{
		m_logStream << obj;
	}
	else
	{
		cout << obj;
	}
	return *this;
}

CPPLog& CPPLog::operator << (unsigned long obj)
{
	CMutexLock lock(m_fileLock);
	
	if (m_logToFile)
	{
		m_logStream << obj;
	}
	else
	{
		cout << obj;
	}
	return *this;
}

CPPLog& CPPLog::operator << (const char* obj)
{
	CMutexLock lock(m_fileLock);
	
	if (m_logToFile)
	{
		m_logStream << obj;
	}
	else
	{
		cout << obj;
	}
	return *this;
}

CPPLog& CPPLog::operator << (string& obj)
{
	CMutexLock lock(m_fileLock);
	
	if (m_logToFile)
	{
		m_logStream << obj;
	}
	else
	{
		cout << obj;
	}
	return *this;
}

//
//template<typename T>
//CPPLog& CPPLog::operator << (const T& obj)
//{
//	CMutexLock lock(m_fileLock);
//	
//	if (m_logToFile)
//	{
//		m_logStream << obj;
//	}
//	else
//	{
//		cout << obj;
//	}
//	return *this;
//}
//
//
//template<typename T>
//CPPLog& CPPLog::operator << (const T* obj)
//{
//	CMutexLock lock(m_fileLock);
//	
//	if (m_logToFile)
//	{
//		m_logStream << obj;
//	}
//	else
//	{
//		cout << obj;
//	}
//	return *this;
//}
//

CPPLog& CPPLog::operator << (CPPLog& (*op)(CPPLog&))
{
	return (*op)(*this);
}


CPPLog& CPPLog::flush()
{
	CMutexLock lock(m_fileLock);

	if (m_logToFile)
	{
		//TODO: 如果自动清理，则判断文件大小，超过尺寸就清理
		m_logStream << std::flush;
	}
	else
	{
		cout << std::flush;
	}
	return *this;
}


bool CPPLog::print(const char* fmt, ...)
{
	// get the input string
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	// print the input to dev
	CMutexLock lock(m_fileLock);
	if (m_logToFile)
	{
		//TODO: 如果自动清理，则判断文件大小，超过尺寸就清理
		m_logStream << buf;
		m_logStream << std::flush;
	}
	else
	{
		cout << buf;
		cout << std::flush;
	}
	
	return true;
}


bool CPPLog::print_nts(const char* fmt, ...)
{
	// get the input string
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	// print the input to dev
	if (m_logToFile)
	{
		//TODO: 如果自动清理，则判断文件大小，超过尺寸就清理
		m_logStream << buf;
		m_logStream << std::flush;
	}
	else
	{
		cout << buf;
		cout << std::flush;
	}
	
	return true;
}

void CPPLog::lock()
{
	pthread_mutex_lock(&m_fileLock);
}

void CPPLog::unlock()
{
	pthread_mutex_unlock(&m_fileLock);
}

CPPLog& endl(CPPLog& log)
{
	log << "\n";
	return log.flush();
}


CPPLog& flush(CPPLog& log)
{
	return log.flush();
}


#ifdef __LOG_DEBUG

int main( int argc, char** argv)
{
	if (argc!=3)
	{
		cerr << "USAGE:\n\t" << argv[0] << " [cout|outfilename] content" << endl;
		return 1;
	}

	if (strcmp(argv[1],"cout") == 0)
	{
		CPPLog log;
		
		log << "=========test begin=========" << endl;
		log << "content: " << flush << argv[2] << endl;
		log.print("format : %s\n", argv[2]);
		log << "==========test end==========" << endl;
	}
	else
	{
		CPPLog log(argv[1]);
		
		log << "=========test begin=========" << endl;
		log << "content: " << flush << argv[2] << endl;
		log.print("format : %s\n", argv[2]);
		log << "==========test end==========" << endl;
	}
}
#endif

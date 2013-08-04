#ifndef COMMON_FUNS_H_
#define COMMON_FUNS_H_
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <stdarg.h>
using namespace std;

#define _trim_right(lpStr)\
        if(lpStr)\
	{\
		char* t;\
		t = lpStr + strlen(lpStr) - 1;\
		while (t >= lpStr && isspace(*t))\
		{\
			*t = 0;\
			t--;\
		}\
	}


//数字转成string
inline string itoa(int i)
{
	char tmp[16];
	sprintf(tmp, "%d", i);
	return tmp;
}


//返回格式化的string类型字符串
inline string STRFormat(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (len >= (int)sizeof(buf))
		len = sizeof(buf) - 1;

	return std::string(buf, len);
}


//互斥锁的封装类
class CMutexLock
{
public:
	CMutexLock(pthread_mutex_t &lock):_lock(lock)
	{
		pthread_mutex_lock(&_lock);
	}
	~CMutexLock()
	{
		pthread_mutex_unlock(&_lock);
	}
private:
	CMutexLock(CMutexLock&);
	const CMutexLock& operator = (const CMutexLock&);
private:
	pthread_mutex_t& _lock;
};


//读写锁的封装类
class RWLock
{
public:
	RWLock(pthread_rwlock_t* lock):_lock(lock){}
	~RWLock(){ pthread_rwlock_unlock( _lock ); }
	
	inline void rdlock() { pthread_rwlock_rdlock( _lock ); };
	inline void wrlock() { pthread_rwlock_wrlock( _lock ); };
	
private:
	pthread_rwlock_t* _lock;
};

#endif

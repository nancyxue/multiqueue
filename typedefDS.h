#ifndef _TYPEDEFDS_H_
#define _TYPEDEFDS_H_

#include <map>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <sys/types.h>
#include "../common/handle.h"
using namespace std;

#define QUEUEDATA_MAX        1024000


typedef struct _SHAREDATA      //共享内存中的内容，包括：数据和数据长度
{
        unsigned int shmLen;
        char shmData[1];
}SHAREDATA;


typedef struct _SHAREPARAM     //共享内存相关参数，包括：共享内存id和地址
{
        int shmid;
	SHAREDATA* shmaddr; 	
}SHAREPARAM;
typedef vector<SHAREPARAM>  SHAREPARAMLIST;


//========================================================================
typedef struct _FORWARDARG
{
	string host;
	string port;
	string query;
	string newid;
}FORWARDARG;
typedef map<unsigned int, FORWARDARG> INDEX_FORWARDARG;          //<index, FORWARDARG>


//======================================================================
typedef map<string, string> FILE_CONF;
typedef multimap<string, string> CMD_ITEM;                       //Unique key matches multi-value
typedef vector<CMD_ITEM> CMD_LIST;
typedef map<unsigned int, CMD_LIST> CMD_CONF;                    //<id, CMD_LIST>


typedef struct _FORKARG
{  
        unsigned int forknum;
        long timethreshold;                                      //Elapsed time threshold 
}FORKARG;  

typedef map<unsigned int, FORKARG> CMD_ID_FORKARG;               //<id, FORKARG>

//===================================================================
typedef struct _CMD_INFO
{
	CMD_ITEM cmdItem;
	bool isPipe;
	PIPE* pPipe;
	string path;
}CMD_INFO;

typedef vector<CMD_INFO> CMD_INFO_LIST;
//====================================================================

typedef struct _IPCParam
{
    CMD_INFO_LIST infoList;
    pid_t pid;              //id对应的其中一个进程的pid          
    int semid;              //id对应的其中一个进程的信号量标识符
    int shmid;              //id对应的其中一个进程的共享内存标识符
    SHAREDATA* shmaddr;     //共享内存的地址
    time_t startTime;       //进程run的起始时间

    time_t reForkStartTime; //重启进程的起始时间,不同于startTime
    unsigned int reForkCnt; //重启进程的次数
}IPCParam;
typedef vector<IPCParam> IPCParamList;

 
typedef struct _IDLEFORK
{
 	  int idleSemid;                  //id对应的进程组中的空闲进程的信号量
 	  IPCParamList paramList;         //id对应的进程组的params 
}IDLEFORK;
 
typedef map<unsigned int, IDLEFORK> ID_IDLEFORK;     //<id, IDLEFORK>

#endif    // End of #ifndef



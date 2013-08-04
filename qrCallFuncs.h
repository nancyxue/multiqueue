#ifndef _QR_CALL_FUNCS_H_
#define _QR_CALL_FUNCS_H_

#include <stdio.h>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <fcntl.h>
#include <signal.h>
#include "../common/arg.h"
#include "../common/luaop.h"
#include "../common/serialize.h"
#include "../common/handle.h"
#include "semUti.h"
#include "typedefDS.h"


using namespace std;

extern int g_term;
extern ID_IDLEFORK g_IDFork;
extern INDEX_FORWARDARG g_index_forwardarg;
extern SHAREPARAMLIST g_ShareParamList;


// Log file
void log(char* message);
FILE* openErrFile();

// Load data related funcs
void loadFile(lua_State* L, FILE_CONF& fileconf);
void loadCmd(lua_State* L, CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, INDEX_FORWARDARG& g_index_forwardarg, bool& isBatchMode);
void loadConf(const char* filename, FILE_CONF& fileconf, CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, INDEX_FORWARDARG& g_index_forwardarg, bool& isBatchMode);

// Pos of file related funcs
unsigned int getPos(const char* filename);
void setPos(const char* filename, unsigned int pos);

// Qlist related funcs
void getQlist(const char* filename, vector<string>& qlist);
void setQlist(const char* filename, const vector<string>& qlist);

// Config related funcs
FILE * openAndSeek(const char* filename, unsigned int pos);
int openAndLock(const char* filename);

size_t strlcpy(char* destiney, const char* source, size_t sizeofdest);


int doData(const char* data, unsigned int len, CMD_INFO_LIST& infoList);

// Signal processing funcs
void onexit(int sig);
void onsigchld(int signo);
void onsigpipe(int signo);
//void onchildexit(int sig);

// Dump info related funcs
void dumpConf(FILE_CONF& fileconf, CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, bool isBatchMode);
void dumpRunTime(const vector<string>& qlist, unsigned int pos, unsigned int batchpos);


//=========================================================================

int writeErrData2File(const SHAREDATA* shmaddr);
int writeEndErrData2File(unsigned int len, char* data);


// Traverse all the forks and stop them when the signal (g_term = 1) emits
int traversalAndStopFork(ID_IDLEFORK& idIdlefork);


// Release all the semaphores and share memrories when the forks exit
int releaseResource();
int releaseSingleSemAndShm(int& semid, SHAREDATA* shmaddr, int& shmid);
int release(int& pid, int& semid, int& shmid, SHAREDATA* shmaddr);


// Load the CMD_INFO_LIST
int loadInfoList1(IPCParam& ipcParam, CMD_LIST& cmdList);
int loadInfoList2(IPCParam& ipcParam, CMD_LIST& cmdList);


//  Reload the CMD_INFO_LIST the some child fork exits
int reloadInfoList(IPCParam& ipcParam, CMD_INFO_LIST& cmdInfoList, CMD_LIST& cmdList);


// Create IPCParam
int createIPCParam(int idleSemid, IPCParam& ipcParam, CMD_INFO_LIST& infoList, bool isNextFile);


// Init and put all the fork info into g_IDFork before doing everything
int initForkList(CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, ID_IDLEFORK& g_IDFork, bool isNextFile);


int childFork(int curSemid, int curShmid, int idleSemid, IPCParam& ipcParam, CMD_LIST& cmdList, /*SHAREPARAMLIST& shareParamList,*/ bool& isNextFile);


int readDataFromFile(bool isRectify, FILE* fp, char* data, unsigned int& len, string& firstFile, string& lastFile, vector<string>& qlist, FILE_CONF& fileconf, CMD_CONF& cmdConf, bool& isBatchMode, unsigned int& batchpos, unsigned int& pos);


// Process when time out
int timeOut(FILE* fp, ID_IDLEFORK& g_IDFork, unsigned int& id, IPCParam& ipcParam, long timeThreshold, string posFileName, unsigned int& pos );


#endif // End of #ifndef


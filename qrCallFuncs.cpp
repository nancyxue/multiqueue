#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h> 
#include "qrCallFuncs.h"

int g_term = 0;
SHAREPARAMLIST g_ShareParamList;

void log(char* message)
{
        FILE* fp = fopen("log.txt", "ab");
	fprintf(fp, "%s\n", message);
	fclose(fp);
}

FILE* openErrFile()
{
        unsigned int count = 0;
	FILE* fpErr = NULL;
        while(!(fpErr = fopen("errorDataFile.txt", "ab+")))
        {
                usleep(100000);
                count ++;
                if (count % 100 == 0)
                {
                        cout << "retry " << count << " open file: " << "errorDataFile.txt" << endl;
                }
                        
        }	
        return fpErr;
}

void loadFile(lua_State* L, FILE_CONF& fileconf)
{
	fileconf.clear();

	lua_getglobal(L, "file");

	if (!lua_istable(L, -1))
	{
		return;
	}

	fileconf["q"] = CLuaOp::lua_getStringItem(L, "q");
	fileconf["p"] = CLuaOp::lua_getStringItem(L, "p");
	fileconf["l"] = CLuaOp::lua_getStringItem(L, "l");
}

FILE* openAndSeek(const char* filename, unsigned int pos)
{
        unsigned int count = 0;
        FILE* fp = NULL;
        while(!(fp = fopen(filename, "r")))
        {
                usleep(100000);
                count ++;
                if (count % 100 == 0)
                {
                        cout << "retry " << count << " open file: " << filename << endl;
                }
        }
        cout << "open data file:    " << filename << endl;

        count = 0;
        while (0 != fseek(fp, pos, SEEK_SET))
        {
                usleep(100000);
                count ++;
                if (count % 100 == 0)
                {
                        cout << "retry " << count << " seek file: " << pos << endl;
                }
        }
        cout << "seek the start pos:  " << pos << endl << endl;

        return fp;
}

int openAndLock(const char* filename)
{
        int fd = open(filename, O_WRONLY | O_CREAT, 0666);
        if (-1 == fd)
        {
                return -12;               // Failed to open file
        }
        if (-1 == lockf(fd, F_TLOCK, 0))
        {
                close(fd);
                return -13;               // Failed to lock file
        }
        return fd;
}

void loadCmd(lua_State* L, CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, INDEX_FORWARDARG& index_forwardarg, bool& isBatchMode)
{
	cmdconf.clear();
	id_forkarg.clear();
	index_forwardarg.clear();

	lua_getglobal(L, "cmd");              //push "cmd"

	if (!lua_istable(L, -1))
	{
		return;
	}

	for (int i=1; ;i++)
	{
		lua_pushnumber(L, i);         //push key i
		lua_gettable(L, -2);          //pop key i, push cmd[i] 
		if (!lua_istable(L, -1))
		{
			break;
		}
                //=============================================================
		unsigned int id = CLuaOp::lua_getIntItem(L, "id");
		unsigned int forkNum = CLuaOp::lua_getIntItem(L, "forkNum");
                unsigned int timeThreshold = CLuaOp::lua_getIntItem(L, "timeThreshold");

                id_forkarg[id].forknum = forkNum;
                id_forkarg[id].timethreshold = timeThreshold;	
	
		for (int j=1; ; j++)
		{
			lua_pushnumber(L, j);      //push key j
			lua_gettable(L, -2);       //pop key j, push cmd[j]
			if (!lua_istable(L, -1))
			{
				lua_pop(L, 1);
				break;
			}

			CMD_ITEM item;

			string opVal = CLuaOp::lua_getStringItem(L, "op");
			item.insert(pair<string, string> ("op", opVal));
			
			if ( "forward" == opVal)
			{      
				FORWARDARG forwardarg;
                                
				for(int k = 1; ; k++)
				{
					lua_pushnumber(L, k);      // push key k
					lua_gettable(L, -2);       // pop key k, push cmd[k]
					if (!lua_istable(L, -1))
					{
						lua_pop(L, 1);     // pop 1 element from the stack top
						break;
					}
					
					string hostval = CLuaOp::lua_getStringItem(L, "host");	
					item.insert(pair<string, string> ("host", hostval));
					forwardarg.host = hostval;

					string portval = CLuaOp::lua_getStringItem(L, "port");
					item.insert(pair<string, string> ("port", portval));
					forwardarg.port = portval;

					string queryval = CLuaOp::lua_getStringItem(L, "query");
					item.insert(pair<string, string> ("query", queryval));
					forwardarg.query = queryval;
						
					string newidval = CLuaOp::lua_getStringItem(L, "newid");
					item.insert(pair<string, string> ("newid", newidval));
					forwardarg.newid = newidval;
				
					pair<unsigned int, FORWARDARG>  bpair(k-1, forwardarg);
					index_forwardarg.insert(bpair);
					
					lua_pop(L, 1);
				}
				
			}
			else if ( "execute" == opVal)
			{
				item.insert(pair<string, string> ("func", CLuaOp::lua_getStringItem(L, "func")));
			}
			else if ( "library" == opVal)
			{
				string module = CLuaOp::lua_getStringItem(L, "module");
				string func = CLuaOp::lua_getStringItem(L, "func");
									  	
				item.insert(pair<string, string> ("module", module ));
				item.insert(pair<string, string> ("func", func ));

				commands.m_librarys[ module ].load( module.c_str(), func.c_str());
			}
			else if ( "batch" == opVal)
			{
				isBatchMode = true;
				string module = CLuaOp::lua_getStringItem(L, "module");
				string func = CLuaOp::lua_getStringItem(L, "func");
											
			  	item.insert(pair<string, string> ("module", module )); 
				item.insert(pair<string, string> ("func", func ));

				commands.m_librarys[ module ].loadBatch( module.c_str(), func.c_str());
			}
			else if ( "pipe" == opVal)
			{
				string path = CLuaOp::lua_getStringItem(L, "path");
				item.insert(pair<string, string> ("path", path ));
			}
			else if ( "batchpipe" == opVal)
			{
				isBatchMode = true;
				
				string path = CLuaOp::lua_getStringItem(L, "path");
				item.insert(pair<string, string> ("path", path ));

				commands.m_pipes[ path ].load( path.c_str() );
			}
			else if ( "serial_lib" == opVal)
			{
				string module = CLuaOp::lua_getStringItem(L, "module");
				string func = CLuaOp::lua_getStringItem(L, "func");
									  	
				item.insert(pair<string, string> ("module", module ));
				item.insert(pair<string, string> ("func", func ));

				commands.m_librarys[ module ].loadSerial( module.c_str(), func.c_str());
			}
			else if ( "serial_fwd" == opVal)
			{
				item.insert(pair<string, string> ("host", CLuaOp::lua_getStringItem(L, "host")));
				item.insert(pair<string, string> ("port", CLuaOp::lua_getStringItem(L, "port")));
				item.insert(pair<string, string> ("query", CLuaOp::lua_getStringItem(L, "query")));
				item.insert(pair<string, string> ("newid", CLuaOp::lua_getStringItem(L, "newid")));
			}

			cmdconf[id].push_back(item);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
}

void loadConf(const char* filename, FILE_CONF& fileconf, CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, INDEX_FORWARDARG& index_forwardarg, bool& isBatchMode)
{
	lua_State* L = luaL_newstate();    // The lua_open function is not mentioned in the lua 5.2 reference manual,replaced by luaL_newstate().

	luaL_openlibs(L);

	luaL_dofile(L, filename);

	loadFile(L, fileconf);

	loadCmd(L, cmdconf, id_forkarg, index_forwardarg, isBatchMode);

	lua_close(L);
}

unsigned int getPos(const char* filename)
{
	unsigned int pos = 0;

	ifstream inFile(filename);
	if (inFile)
	{
	 	inFile >> pos;
		inFile.close();
	}

	return pos;
}

void setPos(const char* filename, unsigned int pos)
{
	string tmpfile = filename;
	tmpfile += ".tmp";
	ofstream outFile(tmpfile.c_str());
	if (outFile)
	{
		outFile << pos;
		outFile.close();
		rename(tmpfile.c_str(), filename);
	}
	chmod(filename, 0666);
}

void getQlist(const char* filename, vector<string>& qlist)
{
	qlist.clear();

	ifstream inFile(filename);
	if (inFile)
	{
		string line;
		while(getline(inFile, line))
		{
			if (line.length())
			{
				qlist.push_back(line);
			}
		}
		inFile.close();
	}
}

void setQlist(const char* filename, const vector<string>& qlist)
{
	ofstream outFile(filename);
	if (outFile)
	{
		vector<string>::const_iterator i;
		for (i = qlist.begin(); i != qlist.end(); ++i)
		{
			outFile << *i << endl;
		}
		outFile.close();
	}
}

void dumpConf(FILE_CONF& fileconf, CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, bool isBatchMode)
{
	FILE_CONF::const_iterator l;
	cout << "file:" << endl;
	for(l=fileconf.begin(); l!=fileconf.end(); ++l)
	{
		cout << "\t\t" << l->first << ": " << l->second << endl;
	}
	cout << endl;

	CMD_CONF::const_iterator i;
	for (i = cmdconf.begin(); i != cmdconf.end(); ++i)
	{
		cout << "id: " << i->first << "\t" 
                     << "forkNum: "<<id_forkarg[i->first].forknum << "\t"
                     << "timeThreshold: "<< id_forkarg[i->first].timethreshold << endl;

		CMD_LIST::const_iterator j;
		for(j=i->second.begin(); j!=i->second.end(); ++j)
		{
			CMD_ITEM::const_iterator k;
			for(k = j->begin(); k != j->end(); ++k)
			{
				cout << "\t\t" << k->first << ": " << k->second << endl;
			}
			cout << endl;
		}
	}

	cout << "isBatchmode: " << (isBatchMode ? "true" : "false") << endl;
	cout << endl;
}

void dumpRunTime(const vector<string>& qlist, unsigned int pos, unsigned int batchpos)
{
	cout << "pos: " << pos << endl;
	cout << "batchpos: " << batchpos << endl;

	cout << "qlist:" << endl;
	for (vector<string>::const_iterator qfile=qlist.begin(); qfile!=qlist.end(); ++qfile)
	{
		cout << "\t" << *qfile << endl;
	}
	cout << endl;
}


//=====================================================================================

int doData(const char* data, unsigned int len, CMD_INFO_LIST& infoList )
{
	if (len < sizeof(ARG_HEAD))
	{
		return -17;                      // Invalid argument                                         
	}

	ARG_HEAD * pArg = (ARG_HEAD*)data;
	
	int bSerial = 0;
	strmap mapItem;
	CMD_INFO_LIST::const_iterator i;
	for (i = infoList.begin(); i != infoList.end(); ++i)
	{
		CMD_ITEM::const_iterator op = i->cmdItem.find("op");
		if (op == i->cmdItem.end())
		{
			continue;
		}
		cout << "OP: " << op->second << endl;
		
		if (op->second == "forward")
		{	
			srand((unsigned)time(NULL));

			unsigned int index = g_index_forwardarg.size();
                 	assert(index > 0);

			bool isPostSucc = false;	
			for(int times = 0; times < index; ++times)
			{
				long num = rand();
				int randNum = num % index;
                                
				string hostTemp = g_index_forwardarg[ randNum ].host;
				string portTemp = g_index_forwardarg[ randNum ].port;
				string queryTemp = g_index_forwardarg[randNum ].query;
    				string newidTemp = g_index_forwardarg[randNum ].newid;

			    	unsigned int uNewId = atoi(newidTemp.c_str());
			    	int retry;
				if (0 == uNewId || pArg->uCmd == uNewId)
				{
					for (retry = 0; retry < 5; ++retry)
					{
			 			if (0 == CArg::PostArg( hostTemp.c_str(), atoi(portTemp.c_str()),
                                                               queryTemp.c_str(), data, len, 3))
						{
							isPostSucc = true;
							break;
						}	 
						sleep(retry);
					}
					if (retry == 5)
					{
						isPostSucc = false;
						continue;
						//return -20;      // Invalid  forward config param in q.conf
					}
					if(true == isPostSucc)
					{
						break;
					} 
				}
				else
				{
					char* newdata = new char[len+1];
					memcpy(newdata, data, len);
				
					ARG_HEAD* p = (ARG_HEAD*)newdata;
					p->uCmd = uNewId;
												
					for (retry = 0; retry < 5; ++retry)
                        		{
                        			if (0 == CArg::PostArg( hostTemp.c_str(), atoi(portTemp.c_str()),
                        					queryTemp.c_str(), data, len, 3))
                        			{
                        				isPostSucc = true;
                        				break;
                        			}
                              			sleep(retry);
                        		}
                        		delete[] newdata;
                        		newdata = NULL;
	
        		                if (retry == 5)
                        		{
                        			isPostSucc = false;
                        			continue;
                         			//return -20;      // Invalid  forward config param in q.conf
                        		}		
                       			if(true == isPostSucc)
                        		{
                        			break;
                        		}
				}
			}
			if(false == isPostSucc)
                        {
                        	return -21;                // Multi-forward failed, something wrong with post-port  
                        }
		}
		else if (op->second == "execute")
		{
			CMD_ITEM::const_iterator func = i->cmdItem.find("func");
			
			if (commands.m_commands.end() == commands.m_commands.find(func->second.c_str()))
			{
				return -22;               // Invalid  execute  config param in q.conf
			}
			
			if (0 != (*(commands.m_commands[func->second.c_str()]))(data, len))
			{
				return -23;               // Failed to func data during execute process
			}
		}
		else if (op->second == "library")
		{
			CMD_ITEM::const_iterator module = i->cmdItem.find("module");
			CMD_ITEM::const_iterator func = i->cmdItem.find("func");
			
			if (0 != commands.m_librarys[module->second].exec(func->second.c_str(), data, len))
			{
				return -24;                // Failed to exec data during library process
			}
		}

		else if (op->second == "pipe")
		{
			CMD_ITEM::const_iterator module = i->cmdItem.find("path");	

			if(0 != i->pPipe->exec(data, len))
			{
				return -25;                // Failed to exec data during pipe process
			}
		}


		else if (op->second == "serial_lib")
		{
			CMD_ITEM::const_iterator module = i->cmdItem.find("module");
			CMD_ITEM::const_iterator func = i->cmdItem.find("func");
			
			if (!bSerial)
			{
				unserialize(data + sizeof(ARG_HEAD), len - sizeof(ARG_HEAD), mapItem);
				bSerial = 1;
			}
			if (0 != commands.m_librarys[module->second].execSerial(func->second.c_str(), data, len, mapItem))
			{
				return -26;                 // Failed to execSerial data during serial_lib process
			}
		}

		if (op->second == "serial_fwd")
		{
			CMD_ITEM::const_iterator host = i->cmdItem.find("host");
			CMD_ITEM::const_iterator port = i->cmdItem.find("port");
			CMD_ITEM::const_iterator query = i->cmdItem.find("query");
			CMD_ITEM::const_iterator newid = i->cmdItem.find("newid");
			unsigned int uNewId = atoi(newid->second.c_str());                       
			
			if (!bSerial)
			{
				unserialize(data + sizeof(ARG_HEAD), len - sizeof(ARG_HEAD), mapItem);
				bSerial = 1;
			}
			
			SerializeResult ret;
			int seriallen = serialize(mapItem, ret);
			
			char* newdata = new char[sizeof(ARG_HEAD) + seriallen];
			memcpy(newdata, data, sizeof(ARG_HEAD));
			memcpy(newdata+sizeof(ARG_HEAD), ret, seriallen);
			ARG_HEAD* p = (ARG_HEAD*)newdata;
			if (uNewId && pArg->uCmd != uNewId)
			{
				p->uCmd = uNewId;
			}
			p->uLen = seriallen;
			
			int retry;
			for (retry = 0; retry < 5; ++retry)
			{
				if (0 == CArg::PostArg(host->second.c_str(),
					atoi(port->second.c_str()),
					query->second.c_str(),
					newdata, sizeof(ARG_HEAD) + seriallen, 3))
				{
					break;
				}
				sleep(retry);
			}
			delete[] newdata;
			newdata = NULL;
			
			if (retry == 5)
			{
				return -27;                  // Failed to post data during serial_fwd  process
			}
		}
	}
	
	return 0;
}




void onexit(int sig)
{
	g_term = 1;
}



void onsigpipe(int signo)
{
	printf("pipe fork quits! \n");
	fflush(stdout);
}



int release( int& pid, int& semid, int& shmid, SHAREDATA* shmaddr )
{
	if(0 < pid)                                         // child fork quits abnormally
        {	
        	pid = 0;
        }
               
	union semun semval;
        semval.val = 0;
        
	if( 0 < semid )
        {
        	//Release sem        
		if( -1 == semctl( semid, 0, IPC_RMID, semval) )
                {
                	perror("IPC_RMID semctl error! \n");
                        //return -19;                        // IPC_RMID shmctl or semctl error
                }
                semid = 0;
        }

        if( 0 < shmid )
        {
		//Release share memory
                if( -1 == shmdt(shmaddr) )
                {
                        perror("detach shm error! \n");
                        //return -18;                        // detach shm error
                }
                //delete(shmaddr);
           	shmaddr = NULL;

        	if( -1 == shmctl( shmid, IPC_RMID, NULL) )
                {
                	perror("IPC_RMID shmctl error! \n");
                        //return -19;                        //IPC_RMID shmctl or semctl error
                }
                shmid = 0;
        }
}


void onsigchld(int signo) 
{  
        pid_t   pid; 
        int     stat;         
        while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        { 
               if(pid > 0)
               {
        	     printf("child %d terminated \n", pid);
		     fflush(stdout);

                     //当被捕获的进程不是被onexit()退出的（ 1.可能因超时退出 2.可能是子进程异常退出 ）
        	     if(1 != g_term && !g_IDFork.empty()) 
                     {
			   sleep(1);
        	     	   //遍历g_IDFork,找到pid在其中对应项的信息并重起一个新的进程
        	     	   ID_IDLEFORK::iterator IFit;
        	     	   for(IFit = g_IDFork.begin(); IFit != g_IDFork.end(); ++IFit)
        	     	   {
        	     	        unsigned int id = IFit->first;
        	     	        int& idleSem = IFit->second.idleSemid;
        	     	        //IPCParamList ipcParamList = IFit->second.paramList;
        	     	        IPCParamList::iterator PLit;
        	     	        for(PLit = IFit->second.paramList.begin(); PLit != IFit->second.paramList.end(); ++PLit)
        	     	        {
        	     	               pid_t prePid = PLit->pid;

        	     	               //Check the prePid of g_IDFork whether it's equal with pid.
        	                       if(pid == prePid)
        	     	               {
                                            time_t reForkEndTime = time((time_t*)NULL);
                                            long duration = reForkEndTime - PLit->reForkStartTime;
                                            unsigned int cnt = ++(PLit->reForkCnt);
                                            if(duration < 60 && cnt >3)
                                            {
                                                  //1 min之内该进程已被重启3次，说明该进程有问题，放弃重启之
                                                  printf("Some error occurs in child  %d when restart it \n", pid);

						  writeErrData2File(PLit->shmaddr);
						  releaseResource();
						  exit(1);
                                            }
                                            if(duration > 60)
                                            {
                                                  PLit->reForkStartTime = reForkEndTime;
                                                  PLit->reForkCnt = 1;
                                            }
					
					    writeErrData2File(PLit->shmaddr);					
					    release(PLit->pid, PLit->semid, PLit->shmid, PLit->shmaddr);
					    int ret = createIPCParam(idleSem, *PLit, PLit->infoList, true);
					    cout << "new pid ==  " << PLit->pid << endl;
					    cout << "new semid == "<< PLit->semid << endl;
					    fflush(stdout);

        	     	                    if(ret < 0 )
        	     	                    {
        	     	                          perror("Create IPCParam failed! \n");
						  int pidTemp = PLit->pid;
						  if(pidTemp > 0)
						  {
							kill(pidTemp, SIGABRT);
						  }

						  writeErrData2File(PLit->shmaddr);
						  releaseSingleSemAndShm(PLit->semid, PLit->shmaddr, PLit->shmid );
        	     	                          exit(1);
        	     	                    }

                                            int flag = v(idleSem, 1);
                                            if(0 > flag)
                                            {
                                                  perror("V operate error! \n");        // Error occurs during  V operate
						  int pidTemp = PLit->pid;
						  if(pidTemp > 0)
						  {
						 	kill(pidTemp, SIGABRT); 
						  }

						  writeErrData2File(PLit->shmaddr);
						  releaseSingleSemAndShm(PLit->semid, PLit->shmaddr, PLit->shmid );
                                                  exit(1);           
                                            }                
        	     	               }	
        	     	                   
        	     	         }
        	           }	              	     	       
                     }
        	     	           
               } 
        }
        return; 
} 



//===============================================================================================



int writeErrData2File(const SHAREDATA* shmaddr)
{
	int fd = open("errorDataFile.txt", O_WRONLY | O_CREAT | O_APPEND);
	if(-1  != fd)
	{
		if((0 == flock(fd, LOCK_EX)) && (-1 != lseek(fd, 0, SEEK_END)) )
		{
			if(sizeof(shmaddr->shmLen) != write(fd, &(shmaddr->shmLen), sizeof(shmaddr->shmLen)))
			{
				perror("fwrite error data len to fpErr failed! \n");
                     		return -28;        //fwrite error data fail
			}

			if(shmaddr->shmLen  != write(fd, shmaddr->shmData, shmaddr->shmLen))
			{
				perror("fwrite error data to fpErr failed! \n");
                		return -28;        //fwrite error data failed
			}

			if(4 != write(fd, "QEND", 4))
			{
				perror("fwrite error 'QEND' to fpErr failed! \n");
                		return -28;        //fwrite error data failed
			}
			flock(fd, LOCK_UN);
		}
		close(fd);
	}
        
	return 0;
}



int writeEndErrData2File(unsigned int len, char* data)
{
	int fd = open("errorDataFile.txt", O_WRONLY | O_CREAT | O_APPEND);
	if(-1 != fd)
	{
		if((0 == flock(fd, LOCK_EX)) && (-1 != lseek(fd, 0, SEEK_END)) )
		{
			//write(fd, &len, sizeof(len));
			write(fd, data, len);
			write(fd, "QEND", 4);
			flock(fd, LOCK_UN);
		}
		close(fd);
	}
        
	return 0;
}



int releaseResource()
{
	if(g_IDFork.empty())
	{
		return 0;
	}
	
	union semun semval;
        semval.val = 0;

	ID_IDLEFORK::iterator IFit; 
	for(IFit = g_IDFork.begin(); IFit != g_IDFork.end(); ++IFit)
	{ 
		unsigned int id = IFit->first;
                IDLEFORK idleFork = IFit->second;
		int idleSem = idleFork.idleSemid;

		IPCParamList::iterator PLit;
		for(PLit = idleFork.paramList.begin(); PLit != idleFork.paramList.end(); ++PLit)
		{	                 
			if(0 < PLit->pid)
			{
				g_term = 1;
				kill(PLit->pid, SIGABRT);
				PLit->pid = 0;
			}

			if( 0 <  PLit->semid )
                      	{
                              	//Release sem
                        	if( -1 == semctl(PLit->semid, 0, IPC_RMID, semval) )
                              	{
                                	perror("IPC_RMID semctl error! \n");
                                	//return -19;                        // IPC_RMID shmctl or semctl error
                              	}
			   	PLit->semid = 0;
			}

			if( 0 < PLit->shmid )
			{
				//Release share memory
                                if( -1 == shmdt(PLit->shmaddr) )
                                {
                                        perror("detach shm error! \n");
                                        //return -18;                        // detach shm error
                                }
                                //delete(PLit->shmaddr);
                                PLit->shmaddr = NULL;

				if( -1 == shmctl(PLit->shmid, IPC_RMID, NULL) )
                              	{
                                        perror("IPC_RMID shmctl error! \n");
                                        //return -19;                        //IPC_RMID shmctl or semctl error
                              	}
				PLit->shmid = 0;
                      	}
		}

		if( 0 < idleSem )
                {
                        if( -1 ==  semctl(idleSem, 0, IPC_RMID, semval))
                        {
                                perror("IPC_RMID semctl error! \n");
                                //return -19;                        // IPC_RMID shmctl or semctl error
                        }
                        idleSem = 0;
                }
	}
        return 0;
}



int releaseSingleSemAndShm(int& semid, SHAREDATA* shmaddr, int& shmid)
{
	union semun semval;
        semval.val = 0;

	if( 0 <  semid )
        {
		//Release sem
                if( -1 == semctl( semid, 0, IPC_RMID, semval) )
                {
                	perror("IPC_RMID semctl error! \n");
                        //return -19;                        // IPC_RMID shmctl or semctl error
               	}
                semid = 0;
        }

        if( 0 < shmid )
        {
		//Release share memory
                if( -1 == shmdt(shmaddr) )
                {
                        perror("detach shm error! \n");
                        //return -18;                        // detach shm error
                }
                //delete(shmaddr);
                shmaddr = NULL;

        	if( -1 == shmctl(shmid, IPC_RMID, NULL) )
                {
                	perror("IPC_RMID shmctl error! \n");
                        //return -19;                        //IPC_RMID shmctl or semctl error
                }
                shmid = 0;
        }
	return 0;	
}


int loadInfoList1(IPCParam& ipcParam, CMD_LIST& cmdList)
{
        ipcParam.infoList.clear();

        CMD_LIST::const_iterator CLit;
        for(CLit = cmdList.begin(); CLit != cmdList.end(); ++CLit)
        {
                CMD_INFO cmdInfo;
                cmdInfo.cmdItem = *CLit;
		cmdInfo.isPipe = false;
                cmdInfo.pPipe = NULL;

                CMD_ITEM::const_iterator opIt = CLit->find("op");
                if (opIt == CLit->end())
                {
                        continue;
                }
                string op = opIt->second;

                if(op == "pipe")
                {
                        CMD_ITEM::const_iterator pathIt = CLit->find("path");
                        if (pathIt == CLit->end())
                        {
                                continue;
                        }
                        string bPath = pathIt->second;
			
			cmdInfo.isPipe = true;
                        cmdInfo.path = bPath;
			
                }
                ipcParam.infoList.push_back(cmdInfo);
        }
        return 0;
}


int loadInfoList2(IPCParam& ipcParam, CMD_LIST& cmdList)
{
	ipcParam.infoList.clear();
	CMD_LIST::const_iterator CLit;
	for(CLit = cmdList.begin(); CLit != cmdList.end(); ++CLit)
	{
		CMD_INFO cmdInfo;
		cmdInfo.cmdItem = *CLit;
		cmdInfo.isPipe = false;
		cmdInfo.pPipe = NULL;

		CMD_ITEM::const_iterator opIt = CLit->find("op");
		if (opIt == CLit->end())
		{
			continue;
		}
		string op = opIt->second;

		if(op == "pipe")
		{
			CMD_ITEM::const_iterator pathIt = CLit->find("path");
			if (pathIt == CLit->end())
			{
				continue;
			}
			string bPath = pathIt->second;

			cmdInfo.pPipe = new PIPE();
			if(NULL == cmdInfo.pPipe)
				return -8;                         // Alloc memory failed 
			cmdInfo.pPipe->load(bPath.c_str());
			cmdInfo.isPipe = true;
			cmdInfo.path = bPath;
		}
		ipcParam.infoList.push_back(cmdInfo);
	}

	return 0;
}


int reloadInfoList(IPCParam& ipcParam, CMD_INFO_LIST&  cmdInfoList, CMD_LIST& cmdList)
{
        CMD_INFO_LIST::iterator CILit;

        for(CILit = cmdInfoList.begin(); CILit != cmdInfoList.end(); ++CILit)
        {
		cmdList.push_back(CILit->cmdItem);

		if( NULL != CILit->pPipe )
		{
			delete(CILit->pPipe);
			CILit->pPipe = NULL;	
		}
		CILit->isPipe = false;

                CMD_ITEM::const_iterator opIt = CILit->cmdItem.find("op");
                if (opIt == CILit->cmdItem.end())
                {
                        continue;
                }
                string op = opIt->second;

                if(op == "pipe")
                {
                        CMD_ITEM::const_iterator pathIt = CILit->cmdItem.find("path");
                        if (pathIt == CILit->cmdItem.end())
                        {
                                continue;
                        }
                        string bPath = pathIt->second;
			CILit->isPipe = true;
                        CILit->path = bPath;
                }
        }

        return 0;
}



int createIPCParam(int idleSemid, IPCParam& ipcParam, CMD_INFO_LIST& infoList, bool isNextFile)
{
	SHAREPARAM shareParam;
	CMD_LIST cmdList;

	//op=pipe时，重启进程
        if( int ret = reloadInfoList(ipcParam, infoList, cmdList) < 0 )
        {
        	return  ret;          //Alloc memory failed
        }

        //创建信号量,并保存至id所对应IPCParam.semid中
        int curSemid = semInit(0);    //创建并打开1个信号量，并对该信号量设置初始值0
        if(-1 == curSemid)
        {
                perror("Creat sem failed! \n");
                return -2;            //Creat sem failed          
        }
        ipcParam.semid = curSemid;
                        
        //创建共享内存,并保存至id所对应IPCParam.shmid中
        int curShmid = shmget(IPC_PRIVATE, QUEUEDATA_MAX, 0666 | IPC_CREAT);    //创建father-child间的共享内存
        if(-1 == curShmid)
        {
                perror("Creat shm failed! \n");
                return -3;            // Create shmid failed
        }                
        ipcParam.shmid = curShmid;

        ipcParam.shmaddr = (SHAREDATA*)shmat(curShmid, NULL, 0);      //将共享内存映射到本进程内存并将内容保存到结构体中
        if((SHAREDATA*)-1 == ipcParam.shmaddr)
        {
                perror("Shmat failed!  \n");
                return -4;            // Shmat failed
        }

        //Initialize structure
        ipcParam.shmaddr->shmLen = 0;
        ipcParam.shmaddr->shmData[0] = '\0';
	
	
	//保存共享内存相关参数，用以在子进程中销毁与子进程无关联的共享内存
        shareParam.shmid = curShmid;
        shareParam.shmaddr = ipcParam.shmaddr;
        g_ShareParamList.push_back(shareParam);
	

        //创建ith子进程,并保存至id对应的IPCParam.pid中
        pid_t curPID = fork();
        if(curPID < 0)
        {
                perror("fork failed! \n");
                return -5;            // Create fork failed
        }
        //==========================================================================================
        else if(0 == curPID)          //子进程负责数据的多进程处理
        {
		int ret =  childFork(curSemid, curShmid, idleSemid, ipcParam, cmdList, isNextFile);
  		if( ret < 0 )
		{
			// Release resource before return if childFork failed
                        // releaseSingleSemAndShm(curSemid, ipcParam.shmaddr, ipcParam.shmid);
                        return -6;    // Error occurs in child fork, func interrupte
                }
                exit(0);
        }  //End of child fork


        //Parent fork
        ipcParam.pid = curPID;
        
        return 0;
}


int initForkList(CMD_CONF& cmdconf, CMD_ID_FORKARG& id_forkarg, ID_IDLEFORK& g_IDFork, bool isNextFile) 
{
	SHAREPARAM shareParam;

	//=======================================================================
	//遍历id_forknum，以获取配置文件中的<id, forkNum>
	CMD_ID_FORKARG::const_iterator IFit;
	for(IFit = id_forkarg.begin(); IFit != id_forkarg.end(); ++IFit)
	{
		unsigned int id = IFit->first;
		unsigned int forkNum = IFit->second.forknum;
                double timeThreshold = IFit->second.timethreshold;    //每个进程的耗时阈值
    
		if(forkNum <= 0)
		{
			cout << "Cur fork cnt should be more than 0!" << endl;
			return  -1;                   // Fork Cnt is invalid
		}
		//=====================================================================
		//创建信号量(idle fork)，保存在其所在id对应的参数idlesemid中, 并进行V操作
		IDLEFORK idleFork;

		int curIdleSemid = semInit(0);
		if(-1 == curIdleSemid)
		{
			return -2;                    // Create idle semid failed
		}
		idleFork.idleSemid = curIdleSemid;
		//=====================================================================
		idleFork.paramList.clear();
		for(unsigned int i = 0; i < forkNum; ++i)
		{
			IPCParam bIPCParam;
			loadInfoList1(bIPCParam, cmdconf[id]);

			//创建信号量,并保存至id所对应IPCParam.semid中
			int curSemid = semInit(0);    //创建并打开1个信号量，并对该信号量设置初始值0
			if(-1 == curSemid)  
			{
				return -2;            // Create semid failed
			}
			bIPCParam.semid = curSemid;

			//创建共享内存,并保存至id所对应IPCParam.shmid中
			int curShmid = shmget(IPC_PRIVATE, QUEUEDATA_MAX, 0666 | IPC_CREAT);    //创建father-child间的共享内存
			if(-1 == curShmid) 
			{
				perror("Creat shm failed!/n");
				return -3;            // Create shmid failed
			}
			bIPCParam.shmid = curShmid;

			bIPCParam.shmaddr = (SHAREDATA*)shmat(curShmid, NULL, 0);      //将共享内存映射到本进程内存并将内容保存到结构体中
			if((SHAREDATA*)-1 == bIPCParam.shmaddr)
			{
				perror("Shmat failed!/n");
				return -4;            // Shmat failed
			}

			//Initialize structure
			bIPCParam.shmaddr->shmLen = 0;
			bIPCParam.shmaddr->shmData[0] = '\0';
			//bIPCParam.shmaddr->term = g_term;
			
			//保存共享内存相关参数，用以在子进程中销毁与子进程无关联的共享内存
			shareParam.shmid = curShmid;
			shareParam.shmaddr = bIPCParam.shmaddr;
			g_ShareParamList.push_back(shareParam);
  
      
			//创建ith子进程,并保存至id对应的IPCParam.pid中
			pid_t curPID = fork();
			if(curPID < 0)
			{
				perror("fork failed! \n");
				return -5;            // Create fork failed 
			}
			//==========================================================================================     
			else if(0 == curPID)          //子进程负责数据的多进程处理
			{ 
				int ret = childFork(curSemid, curShmid, curIdleSemid, bIPCParam, cmdconf[id], isNextFile);
				if(ret < 0)
				{
					// Release resource before return if childFork failed 
				        // releaseSingleSemAndShm(curSemid, bIPCParam.shmaddr, bIPCParam.shmid)	
					fflush(stdout);
					cout << "child fork: ret =  "<< ret << endl;
					fflush(stdout);
					return -6;    // Error occurs in child fork, func interrupted
				}
				exit(0);

			}  //End of child fork 

			//Parent fork
			bIPCParam.pid = curPID;
             
                        bIPCParam.reForkStartTime = time((time_t*)NULL);
                        bIPCParam.reForkCnt = 0;

			idleFork.paramList.push_back(bIPCParam);

			int flag = v(idleFork.idleSemid, 1);   
			if(0 > flag)
			{
				perror("V operate error! \n");
				return -7;           // Error occurs during  V operate  
			}
		}
    		pair<unsigned int, IDLEFORK>  bpair(id, idleFork);
		g_IDFork.insert(bpair);
  }
  return 0;
}


int childFork(int curSemid, int curShmid, int idleSemid, IPCParam& ipcParam, CMD_LIST& cmdList, bool& isNextFile)
{
	// Delete shmaddr which not belonged to current fork 
	SHAREPARAMLIST::iterator SPLit;
	for(SPLit = g_ShareParamList.begin(); SPLit != g_ShareParamList.end(); ++SPLit)
	{
		if(SPLit->shmid != curShmid)
		{
			if( -1 == shmdt(SPLit->shmaddr) )
                        {
                                perror("detach shm error! \n");
                                //return -18;                        // detach shm error
                        }
                        SPLit->shmaddr = NULL;
		}
	}

	// Move the pipe loading into each child fork to avoid that the pipefd can't be released.
	loadInfoList2(ipcParam, cmdList);

	while(1)
	{
		int flag = p(curSemid);          
		if(flag)
		{
			perror("P operate error! \n");
			return -9;                  //  Error occurs during  P operate
		}
     
		if(ipcParam.shmaddr->shmLen != 0)
		{
			int ret = doData(ipcParam.shmaddr->shmData, ipcParam.shmaddr->shmLen, ipcParam.infoList);
			cout << "RET: " << ret << endl;			
		 	fflush(stdout);
	
			if(ret == -17)
			{
				return ret;
			}

			if(ret >= -27 && ret <= -21)       // doData fail
			{				
				// If failed, write the error data to errorDataFile.txt

				writeErrData2File(ipcParam.shmaddr);
			}

			if(ret == -25)
                        {
                                //usleep(100000);
                                loadInfoList2(ipcParam, cmdList);
                        }
		}

		// Set semVal to 0
                union semun semval;
                semval.val = 0;
                int ret = semctl(curSemid, 0, SETVAL, semval);

		flag = v(idleSemid, 1);
                if(0 > flag)
                {
                	cout<< "idleSemid = " << idleSemid << endl;
                        fflush(stdout);
                        perror("V operate error! \n");
                        return -7;         // Error occurs during  V operate
                }
	 }//End of while(1)
}		    
	
	
int readDataFromFile(bool isRectify, FILE* fp, char* data, unsigned int& len, string& firstFile, string& lastFile, vector<string>& qlist, FILE_CONF& fileconf, CMD_CONF& cmdConf, bool& isBatchMode, unsigned int& batchpos, unsigned int& pos)
{
	int ret = 0;
	unsigned int count = 0;
	
	while (1 != (ret = fread(&len, 4, 1, fp)))
	{ 
		if (g_term)
		{
			time_t now = time(0);
			cout << ctime(&now) << "3 - Killed!" << endl << endl;

			return -14;                        // Interrupt signal
		}
		
	
		if (feof(fp))
		{	//文件末尾了
			if(firstFile != lastFile)
			{	//当前文件不是最后一个文件，从列表中删除第一个文件，然后将文件指针设置为0
				cout<< "firstFile != lastFile" << endl;
				qlist.erase(qlist.begin());
				pos = 0;
				setPos(fileconf["p"].c_str(), pos);
				setQlist(fileconf["q"].c_str(), qlist);

				return -15;
			}
			else
			{			    	
				if (isBatchMode)
				{
					//batchpos = batchFile(fp, pos, batchpos, cmdConf);
				}
			}
		}
			
		usleep(100000);
		count ++;
		if (count % 100 == 0)
		{
			time_t now = time(0);
			cout << ctime(&now) << "retry " << count << " read len!" << endl << endl;
		}
	}   // End of while

			
	time_t now = time(0);
	cout << ctime(&now) << "------ Start read data -----\nLEN: " << len << endl;
		      
	//读取数据内容，处理数据内容
	if (len)
	{
		count = 0;
		while (1 != (ret = fread(data, len, 1, fp)))
		{
			usleep(1000000);
			count ++;
			if (count % 100 == 0)
			{
				time_t now = time(0);
				cout << ctime(&now) << "retry " << count << " read data!" << endl << endl;

				if (count >= 360000)
				{  
					sleep(1);
					break;
				}
			}
		}
		if (ret != 1)
		{
			return -16;                 // Failed to read data from file
		}
	}
		
	//读取数据包结尾
	unsigned int cnt = 0;
	char szEnd[4] = {0};
	while (1)
	{
		if (1 == fread(szEnd, sizeof(szEnd), 1, fp))
		{
			if (0 == memcmp(szEnd, "QEND", sizeof(szEnd)))
			{
			 	//cout << "read the end of file has completed!" << endl; 	
				break;
			}
			else
			{
				fseek(fp, 1-sizeof(szEnd), SEEK_CUR);
			}
		}
		else
		{
			return -16;  		     
			//usleep(100000);	    // Failed to read data from file
		}
	}

	return 0;
}
	


int timeOut(FILE* fp, ID_IDLEFORK& g_IDFork, unsigned int& id, IPCParam& ipcParam, long timeThreshold, string posFileName, unsigned int& pos )
{	      
	time_t end = time((time_t*)NULL);              //cur time
        long duration = end - ipcParam.startTime;

        if(duration > timeThreshold)
        {	
		//==========================================================================================================
                //该子进程在处理数据unit时超时，则将该子进程共享内存中的数据unit写到errorDataFile.txt文件中，同时kill该子进程
                //ps: 此时PLit->shmaddr里保存的仍然是上次的数据内容
		
		writeErrData2File(ipcParam.shmaddr);	
                
                //越过错误的数据unit设置读文件的新的起始位置
                pos = ftell(fp);        //更新读文件的位置
                pos += sizeof(ipcParam.shmaddr->shmLen) +  ipcParam.shmaddr->shmLen + 4; 
                setPos(posFileName.c_str(), pos);
                
       		if(0 < ipcParam.pid)
		{        
			kill(ipcParam.pid, SIGABRT);
			//ipcParam.pid = 0;
		}
		                                  
                union semun semval;
                semval.val = 0;

		if( 0 < ipcParam.semid )
		{ 
                	//Release sem
                	if( -1 == semctl( ipcParam.semid, 0, IPC_RMID, semval) )
                	{
                        	perror("IPC_RMID semctl error! \n");
                        	//return -19;                        // IPC_RMID shmctl or semctl error
                	}
			ipcParam.semid = 0;
		}

		if( 0 < ipcParam.shmid )
		{			
			//Release share memory
                        if( -1 == shmdt(ipcParam.shmaddr) )
                        {
                                perror("detach shm error! \n");
                                //return -18;                        // detach shm error
                        }
			ipcParam.shmaddr = NULL;

			if( -1 == shmctl( ipcParam.shmid, IPC_RMID, NULL) )
                	{
                        	perror("IPC_RMID shmctl error! \n");
                        	//return -19;                        //IPC_RMID shmctl or semctl error
                	}
                	ipcParam.shmid = 0;
		}                          
                                                 
                return 0;  //超时情况处理完毕后，继续遍历                                          
        }
        return 1;    //未超时的情况下，也继续遍历
}

#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "typedefDS.h"
#include "semUti.h"
#include "qrCallFuncs.h"


ID_IDLEFORK g_IDFork;
INDEX_FORWARDARG g_index_forwardarg;


int main(int argc, char** argv)
{
  if (argc != 2)
        {
                cout << "Usage: " << argv[0] << " <conffile>" << endl;
                return EINVAL;             //Invalid argument
        }

	//处理kill命令和CTRL C退出命令
        signal(SIGTERM, onexit);
        signal(SIGINT,  onexit);
        signal(SIGCHLD, onsigchld);
	signal(SIGPIPE, onsigpipe);

	srand((unsigned)time(NULL));

        //本块内存用来存放每次从数据文件中取出的数据unit,在程序运行期间不释放直至主进程退出由OS释放。
	char* data = new char[QUEUEDATA_MAX];
	assert(data);

	//加载配置
	FILE_CONF fileconf;
	CMD_CONF cmdconf;
        CMD_ID_FORKARG id_forkarg;

	bool isBatchMode = false;          //标记是否进行批处理
	loadConf(argv[1], fileconf, cmdconf, id_forkarg, g_index_forwardarg, isBatchMode);

	//=====================================================================

        int ret = initForkList(cmdconf, id_forkarg, g_IDFork, true);

	//输出配置信息
	dumpConf(fileconf, cmdconf, id_forkarg, isBatchMode);
	
	//加载队列文件名列表
	vector<string> qlist;
	getQlist(fileconf["q"].c_str(), qlist);
	if (!qlist.size())
	{
		cout << "Error:  exist no list!" << endl;
		releaseResource();
		return ENXIO;              // Configure error
	}
	
	//对文件加锁，保证只有一个实例运行
	int fd = openAndLock(fileconf["l"].c_str());
	if (0 >  fd)
	{
		cout << "Error:  open or lock file: " << fileconf["l"] << " failed!"<< endl;
		releaseResource();
		return ENOENT;             //Failed to open or lock file                   
	}


	//获取队列最后一个文件名，这个文件是被写文件
	string lastfile = qlist[qlist.size()-1];

	int flag = 0;
	unsigned int pos, batchpos = 0;
	
	//循环遍历qlist中所有队列文件名
	bool bNextFile = true;	          //标记是否继续处理qlist中的下一个数据文件
	for ( ; bNextFile; )
	{
		string firstfile = qlist[0];
		pos = getPos(fileconf["p"].c_str());
		batchpos = pos;
		bool isRectify = false;
	  
		FILE* fp = openAndSeek(firstfile.c_str(), pos);    //打开文件并定位到指定位置

		//===========================================================================
	
        	while(1)
		{
			if (g_term)
			{
				time_t now = time(0);
				cout << ctime(&now) << "1 - Killed!" << endl << endl;
				//遍历各子进程进行V操作以使之接收退出信号
				releaseResource();				
				bNextFile = false;
				break;
			}
			if (firstfile == lastfile && pos > 1000 * 1024 * 1024)
			{
				//当前文件是最后一个文件，并且文件大于指定尺寸，将文件改名，重新生成队列文件名列表
				char timestamp[32] = {0};
				sprintf(timestamp, ".%ld", time(0));
				string newfile = lastfile + timestamp;
				rename(lastfile.c_str(), newfile.c_str());
				//创建lastfile 666 权限，防止写队列文件失败的情况
				std::ofstream curFile(lastfile.c_str(), std::ios::binary|std::ios::app);
				chmod(lastfile.c_str(), 0666);
				curFile.close();
				qlist.clear();
				qlist.push_back(newfile);
				qlist.push_back(lastfile);
				setQlist(fileconf["q"].c_str(), qlist);
				dumpRunTime(qlist, pos, batchpos);
				break;
			}

			unsigned int len = 0;
			int ret = 0;
			if( (0 > (ret = readDataFromFile(isRectify, fp, data, len, firstfile, lastfile, qlist, fileconf, cmdconf, isBatchMode, batchpos, pos))) )
			{
				if(-15 == ret)
				{
					break;
				}	
				
				if(-16 == ret)
				{
                                        fflush(stdout);
					writeEndErrData2File(len, data);
                        	        pos = ftell(fp);       //更新读文件的位置
                	                setPos(fileconf["p"].c_str(), pos);
        	                        cout << "new pos:   " << pos << endl << endl;
				}
				continue;	
			}
			  
			//从data unit中获取id
			if (len < sizeof(ARG_HEAD) && len > 0)
			{
             			fflush(stdout);
				writeEndErrData2File(len, data);

                                pos = ftell(fp);       //更新读文件的位置
                                setPos(fileconf["p"].c_str(), pos);
                                cout << "new pos:   " << pos << endl << endl;
                                continue;				
			}

			ARG_HEAD* pArg = (ARG_HEAD*)data;
			if (pArg->uVersion != 1)
			{
				isRectify = true;
				int rewindLen = (-1) * (len + 8); 
				fseek(fp, rewindLen, SEEK_CUR);
				readDataFromFile(isRectify, fp, data, len, firstfile, lastfile, qlist, fileconf, cmdconf, isBatchMode, batchpos, pos);		
				isRectify = false; 

                                fflush(stdout);
				writeEndErrData2File(len, data);
				
                                pos = ftell(fp);       //更新读文件的位置
                                setPos(fileconf["p"].c_str(), pos);
                                cout << "new pos:   " << pos << endl << endl;
                                continue;              
			}
			if ( (pArg->uLen + sizeof(ARG_HEAD)) != len)
			{
                                fflush(stdout);
				writeEndErrData2File(len, data);
  
                                pos = ftell(fp);       //更新读文件的位置
                                setPos(fileconf["p"].c_str(), pos);
                                cout << "new pos:   " << pos << endl << endl;
                                continue;	
			}

			cout << "CMD: " << pArg->uCmd << endl;
			unsigned int id = pArg->uCmd;
			  
			//===========================================================================
			
                        flag = p(g_IDFork[id].idleSemid);               
			if(flag)
			{
				perror("P operate error! \n");
				releaseResource();
				bNextFile = false;
                                break; 
			}
        
			bool breakLoop = false;
			IPCParamList::iterator PLit;
			for(PLit = g_IDFork[id].paramList.begin(); PLit != g_IDFork[id].paramList.end(); ++PLit)
			{
				if( PLit->semid == 0 )
				{
					continue;
				}

				int ret;
			  	union semun semarg;
				semarg.val = 0;
				ret = semctl(PLit->semid, 0, GETVAL);      //检测此时该信号量是否空闲
				if(ret == -1)
				{
					fflush(stdout);
					cout << "curSemid == " << PLit->semid <<endl;
					fflush(stdout);
					perror("semctl failed!\n");
					fflush(stdout);

					continue;					
				}
				else if(ret == 0)
				{
				        if(len > QUEUEDATA_MAX-9)
						len = QUEUEDATA_MAX - 10;
					memcpy(PLit->shmaddr->shmData, data, len); //将此次fread到的data放到father-child间的共享内存
					PLit->shmaddr->shmData[len] = '\0';
					PLit->shmaddr->shmLen = len;


				        flag = v(PLit->semid,2);
					if(0 > flag)
					{
						perror("V operate faild! \n");
						continue;
					} 

                                  
                                        //记录该子进程run的起始时间
                                        //gettimeofday(&start, NULL);
                                        PLit->startTime = time((time_t*)NULL);


					pos = ftell(fp);       //更新读文件的位置   
					setPos(fileconf["p"].c_str(), pos);
					cout << "now pos:   " << pos << endl << endl;
					break;
				}
                                else if(ret > 0)              //若该子进程仍在处理共享内存中的数据
                                {
                                        if( 0 <= timeOut(fp, g_IDFork, id, *PLit, id_forkarg[id].timethreshold, fileconf["p"], pos) )
                                        {
                                                 continue;
                                        }
                                        else
                                        {
						 releaseResource();
						 breakLoop = true;
                                                 break;
                                        }
                                }

			}  // End of for  

			//处理batchmode
			if (isBatchMode)
			{
				if (pos - batchpos > 1024 * 1024 * 10)
				{
					//batchpos = batchFile(fp, pos, batchpos, cmdconf);
				}
			}

			if(true == breakLoop)
                        {
                                bNextFile = false;
                                break;
                        }

		}  // End of while(1)

		if (isBatchMode)
		{
			//batchpos = batchFile(fp, pos, batchpos, cmdconf);
		}
		//关闭文件
		fclose(fp);
	}

	//关闭锁文件
	close(fd);
	return 0;
}


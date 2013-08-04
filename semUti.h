#include "semUti.h"

int semInit(int val)
{
    union semun semval;
    semval.val = val;
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if(semid == -1)
    {
        perror("in semInit(), semget failed!");
        return -1;
    }
    
    int ret;
    ret = semctl(semid, 0, SETVAL, semval);  
    if(ret == -1)
    {
        perror("in semInit(), semctl failed!");
        return -1;
    }
    return semid;
}

int p(int semid)
{
    struct sembuf op;
    op.sem_num = 0;    //信号量标号
    op.sem_op  = -1;   //执行的操作:减1
    op.sem_flg = 0;   

    int ret;
    while(ret = semop(semid, &op, 1));  //操作成功返回0,则跳出while; 否则停在这儿。即：semop出错返回-1时，不再往下走。
    return ret;
}

int v(int semid, int val)
{
    struct sembuf op;
    op.sem_num = 0;   //信号量标号
    op.sem_op  = val;   //执行的操作:加1 
    op.sem_flg = 0;  

    int ret;
    ret = semop(semid, &op, 1);
    return ret;
}

int p_NonBlock(int semid)
{
    struct sembuf* op;
    op->sem_num = 0;    //信号量标号
    op->sem_op  = -1;   //执行的操作:减1
    op->sem_flg = IPC_NOWAIT;    

    int ret;
    ret = semop(semid, op, 1);       //操作成功返回0,否则若操作不成功,则不等待继续下面的操作
    return ret;
}

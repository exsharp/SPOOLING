//
// Created by zfliu on 2015/12/29.
//

#ifndef SPOOLING_REQUEST_H
#define SPOOLING_REQUEST_H

#define REQUEST_BLOCK_LEN 10

typedef struct{  //请求块
    int reqname;
    int length;
}_ReqBlock;

typedef struct{ //指针，请求块已用
    int n_begin;  //因为请求块作为环形队列，故这里是开始,从0开始
    int n_length; //这里是请求块已用个数
    int n_end;
}ReqPtr;

typedef struct{
    _ReqBlock reqBlock[REQUEST_BLOCK_LEN]; //请求块
    ReqPtr reqPtr;//记录请求块已用信息
    char buffer[REQUEST_BLOCK_LEN][30]; //输出井
}ReqBlock;

typedef struct{
    int pid;
    int status;
}PCB;

typedef struct{
    ReqBlock *global;
    PCB *pcb;
    int *t;
    PCB *main_pcb;
}ReqParams;

typedef struct{
    ReqBlock *global;
    PCB *pcb;//要调用的PCB列表
    PCB *main_pcb;//输出的进程的PCB
    int user_num;//request进程数量
    int fd;
    pthread_mutex_t *mutex;
}PrtParams;

void request(ReqParams *params);
void print(PrtParams *params);

#endif //SPOOLING_REQUEST_H

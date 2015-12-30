//
// Created by zfliu on 2015/12/29.
//
#include <stdlib.h>
#include <stdio.h>
#include "request.h"

void request(ReqParams *params){

    ReqBlock *global = params->global;
    PCB *pcb = params->pcb;
    PCB *main_pcb = params->main_pcb;
    int *t = params->t;

    if (global->reqPtr.n_length == REQUEST_BLOCK_LEN){
        //没用空闲的请求块
        //转调度程度
        pcb->status = 1;
        return;
    }

    //还有剩余的空闲块
    int empty_pos = (global->reqPtr.n_begin + global->reqPtr.n_length)%REQUEST_BLOCK_LEN;
    //空闲的位置

    int i,tmp=0;
    for (i=0;i<10;i++) {
        tmp = (rand() % 9) + 1;  //前10个让它不为0
        global->buffer[empty_pos][i] =(char)(tmp+'0');
    }
    for (i=10;i<30 && tmp!=0;i++){
        if (i==29){
            global->buffer[empty_pos][29] = '0';
            break;
        }
        tmp = rand()%10;
        global->buffer[empty_pos][i] = (char)(tmp+'0');
    }

    global->reqBlock[empty_pos].reqname = pcb->pid;
    global->reqBlock[empty_pos].length = i-1;
    //global->reqPtr.n_length = global->reqPtr.n_length ++;
    global->reqPtr.n_length ++ ;
    global->reqPtr.n_length = global->reqPtr.n_length % REQUEST_BLOCK_LEN;

    //剩余文件减1
    (*t)--;
    if ((*t)==0){
        pcb->status = 3;
    }

/*    status=0 为可执行态；
    status=1 为等待状态1，表示请求输出块用完，请求输出的用户进程等待；
    status=2 为等待状态2， 表示输出井空，SPOOLING输出进程等待；
    status=3 为结束态，进程执行完成。*/

    //设置为可执行态
    if (main_pcb->status == 2){
        main_pcb->status = 0;
    }
    return;
}

void print(PrtParams *prtParams){
    ReqBlock *global = prtParams->global;
    PCB *pcb = prtParams->pcb;
    PCB *main_pcb = prtParams->main_pcb;
    int user_num = prtParams->user_num;

    //空闲请求输出块数=10？
    if ( REQUEST_BLOCK_LEN - global->reqPtr.n_length == REQUEST_BLOCK_LEN){
        int i = 0;
        int finish_req = 1;
        for (;i<user_num;i++){
            if (pcb[i].status!=3){
                finish_req = 0;
            }
        }
        //两个请求输出都结束
        if (finish_req==1){
            //输出进程结束
            main_pcb->status = 3;
            return;
        }else{
            //输出进程等待
            main_pcb->status = 2;
            return;
        }
    }

    FILE *file;
    if((file=fopen("D:\\WorkSpace\\SPOOLING\\test.txt","a+"))==NULL)
    {
        printf("can't open file");
    }

    //空闲请求块!=10
    int i = 0;
    int loop_count = global->reqPtr.n_length;
    int loop_begin = global->reqPtr.n_begin;
    for (;i<loop_count;i++){
        int now = (loop_begin + i) % REQUEST_BLOCK_LEN;
        int succ = (int)fwrite(global->buffer[now],sizeof(char),(size_t)global->reqBlock[now].length,file);
        fwrite("\n",sizeof(char),1,file);
        printf("%d\n",succ);

        //环形队列减1
        global->reqPtr.n_begin++;
        global->reqPtr.n_length--;
    }
    fclose(file);

    for (i=0;i<user_num;i++){
        if (pcb[i].status==1){
            pcb[i].status = 0;
        }
    }

    /*    status=0 为可执行态；
    status=1 为等待状态1，表示请求输出块用完，请求输出的用户进程等待；
    status=2 为等待状态2， 表示输出井空，SPOOLING输出进程等待；
    status=3 为结束态，进程执行完成。*/

    return;
}

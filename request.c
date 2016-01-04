#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "request.h"

//1000微妙是1毫秒
//1秒是1000毫秒
int sleep_time=500;

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
    //int empty_pos = (global->reqPtr.n_begin + global->reqPtr.n_length)%REQUEST_BLOCK_LEN;
    int empty_pos = global->reqPtr.n_end;
    //空闲的位置

    int i,tmp=0,sum;
    sum = (rand()%24)+6;  //文件长度1~29  最后一位放0
    for (i=0;i<sum;i++) {
        tmp = (rand() % 9) + 1;  //让它不为0
        global->buffer[empty_pos][i] =(char)(tmp+'0');
    }
    global->buffer[empty_pos][i] = '0' ;

    global->reqBlock[empty_pos].reqname = pcb->pid;
    global->reqBlock[empty_pos].length = i;
    global->reqPtr.n_length ++ ;
    global->reqPtr.n_end ++ ;
    if (global->reqPtr.n_length!=REQUEST_BLOCK_LEN){
        global->reqPtr.n_length = global->reqPtr.n_length % REQUEST_BLOCK_LEN;
        global->reqPtr.n_end = global->reqPtr.n_begin + global->reqPtr.n_length;
    }

    printf("    占用空闲块位置:%d,文件长度:%d\n    文件内容:",empty_pos,i);
    int j = 0;
    for (j;j<i-1;j++)
        printf("%c",global->buffer[empty_pos][j]);
    printf("\n");
    printf("    请求快现状:空闲请求块数量:%d\n",REQUEST_BLOCK_LEN - global->reqPtr.n_length);

    //剩余文件减1
    (*t)--;
    if ((*t)==0){
        pcb->status = 3;
    }

/*  status=0 为可执行态；
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
    int fd = prtParams->fd;

    //空闲请求输出块数=10？
    if ( global->reqPtr.n_length == 0){
        printf("    没有请求块被占用\n");
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
            printf("    所有进程输出完毕\n");

            //释放锁
            pthread_mutex_unlock(prtParams->mutex);
            return;
        }else{
            //输出进程等待
            printf("    输出进程等待输出\n");
            main_pcb->status = 2;

            //释放锁
            pthread_mutex_unlock(prtParams->mutex);
            return;
        }
    }

    //空闲请求块!=10
    int i = 0;
    int loop_count = global->reqPtr.n_length;
    int loop_begin = global->reqPtr.n_begin;
    for (;i<loop_count;i++){
        int now = (loop_begin + i) % REQUEST_BLOCK_LEN;

        printf("    输出%d号进程内容(长度:%d):",global->reqBlock[now].reqname,global->reqBlock[now].length);
        int j = 0;
        int len = global->reqBlock[now].length-1;

        for (j;j<len;j++){
            printf("%c",global->buffer[now][j]);
        }
        printf("\n");
        lockf(1,0,0);
        for (j=0;j<len;j++){
            write(fd,&global->buffer[now][j],1);
            usleep(1000*sleep_time);
        }
        write(fd,"\n",1);


        //环形队列减1
        global->reqPtr.n_begin++;
        global->reqPtr.n_length--;

        break; //只输出一个文件
    }

    for (i=0;i<user_num;i++){
        if (pcb[i].status==1){
            pcb[i].status = 0;
        }
    }

    /*    status=0 为可执行态；
    status=1 为等待状态1，表示请求输出块用完，请求输出的用户进程等待；
    status=2 为等待状态2， 表示输出井空，SPOOLING输出进程等待；
    status=3 为结束态，进程执行完成。*/

    //释放锁
    pthread_mutex_unlock(prtParams->mutex);
    return;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "request.h"

#define PROC_COUNT 2

void init(ReqBlock *req,PCB *pcb,PCB *main_pcb){
    req->reqPtr.n_length=0;
    req->reqPtr.n_begin=0;
    int i;
    for (i = 0;i<=2;i++){
        pcb[i].pid=i+1;
        pcb[i].status=0;
    }
    main_pcb->status=2;
}

int main() {

    srand((unsigned int)time(NULL));

    // 文件个数
    int file[PROC_COUNT]={10,15};

    ReqBlock reqBlock;
    PCB pcb[PROC_COUNT];
    PCB main_pcb;
    init(&reqBlock,pcb,&main_pcb);

    ReqParams reqParams;
    reqParams.global = &reqBlock;
    reqParams.main_pcb = &main_pcb;

    int loop_count=0;
    for (;;){
        loop_count++;
        printf("%d\n",loop_count);

        double run = (double)rand()/RAND_MAX;
        if (run < 0.4 && pcb[0].status==0) {
            //请求进程1
            reqParams.pcb = &pcb[0];
            reqParams.t = &file[0];
            request(&reqParams);
        } else if (run < 0.8 && pcb[1].status==0){
            //请求进程2
            reqParams.pcb = &pcb[1];
            reqParams.t = &file[1];
            request(&reqParams);
        } else if (run < 1 && main_pcb.status==0){
            //输出进程
            PrtParams prtParams;
            prtParams.user_num = PROC_COUNT;
            prtParams.global = &reqBlock;
            prtParams.pcb = pcb;
            prtParams.main_pcb =  &main_pcb;
            print(&prtParams);
        }

        //判断所有进程都运行结束
        int i = 0;
        int flag = 0;
        for (i=0;i<PROC_COUNT;i++){
            if (pcb[i].status != 3){
                flag = 1;
            }
        }
        if (flag == 0){
            if (main_pcb.status == 3){
                break;
            }
        }
    }

    printf("ALL PROCESS FINISH");
    return 0;
}
/*#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
pthread_t main_tid;

void func(int *a)
{
    while(1){
        if ((*a) == 1){
            printf("HelloWorld");
            break;
        }
    }
}
int main()
{
    int a = 0;
    pthread_create(&main_tid, NULL, &func, &a); //创建线程
    printf("0");
    a=1;
    printf("1");

    sleep(2);
    return 0;
}*/

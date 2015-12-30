#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#include "request.h"

#define PROC_COUNT 3
#define FILE_COUNT 7,21,15

//打印机每个字符打印时间
extern int sleep_time;

void init(ReqBlock *req,PCB *pcb,PCB *main_pcb){
    req->reqPtr.n_length=0;
    req->reqPtr.n_begin=0;
    int i;
    for (i = 0;i<=PROC_COUNT;i++){
        pcb[i].pid=i+1;
        pcb[i].status=0;
    }
    main_pcb->status=2;
}

int main() {

    srand((unsigned int)time(NULL));
    sleep_time = 1000*1;

    //文件参数
    int fd;
    fd = open("D:\\WorkSpace\\SPOOLING\\test.txt",O_NOCTTY|O_WRONLY|O_TRUNC|O_APPEND,0660);

    // 文件个数
    int file[PROC_COUNT]={FILE_COUNT};

    ReqBlock reqBlock;
    PCB pcb[PROC_COUNT];
    PCB main_pcb;
    init(&reqBlock,pcb,&main_pcb);

    //“打印机”线程锁
    pthread_t tid;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    //请求进程的参数列表
    ReqParams reqParams;
    reqParams.global = &reqBlock;
    reqParams.main_pcb = &main_pcb;

    //Print进程的参数列表
    PrtParams prtParams;
    prtParams.user_num = PROC_COUNT;
    prtParams.global = &reqBlock;
    prtParams.pcb = pcb;
    prtParams.main_pcb =  &main_pcb;
    prtParams.fd = fd;
    prtParams.mutex = &mutex;


    int loop_count=0;
    for (;;){
        sleep(1);
        loop_count++;
        printf("调度次数:%d:\n",loop_count);

        double run = (double)rand()/RAND_MAX;
        if (run < 0.8){
            double pro = 0.8 / PROC_COUNT;
            int count = (int)(run / pro);
            printf("  %d号请求进程:\n",count);
            if (pcb[count].status == 0){
                reqParams.pcb = &pcb[count];
                reqParams.t = &file[count];
                request(&reqParams);
            }
            if (pcb[count].status==3){
                printf("    进程为结束态\n");
            }
            if (pcb[count].status==1){
                printf("    没有空闲的请求块\n");
            }
            printf("  进程现况:status:%d,file_left:%d\n",pcb[count].status,file[count]);
        } else if (run > 0.8 && main_pcb.status==0){
            printf("  输出进程:\n");
            //输出进程
            int ret = pthread_mutex_trylock(&mutex);
            if (ret == EBUSY){
                printf("    “打印机”正忙");
            }else{
                lockf(1,1,0);
                pthread_create(&tid,NULL,&print,&prtParams);
                //print(&prtParams);
            }
            printf("  进程现况:status:%d\n",main_pcb.status);
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
    pthread_join(tid,NULL);
    printf("\n** ALL PROCESS FINISH **");
    return 0;
}
/*
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
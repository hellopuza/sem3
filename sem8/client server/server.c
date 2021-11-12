#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"

void makeop(int n);
void* solution (void* arg);

int semid;
int msqid;

int main()
{
    char pathname[] = "file";
    key_t key;
    int len;

    if ((key = ftok(pathname,0)) < 0)
    {
        printf("Can\'t generate key\n");
        exit(-1);
    }

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0)
    {
        printf("Can\'t get msqid\n");
        exit(-1);
    }

    if ((semid = semget (key, 1, 0666 | IPC_CREAT)) < 0)
    {
        printf ("Can\'t get semid\n");
        exit (-1);
    }

    makeop(get_nprocs() - 1);

    pthread_t thid;
    struct MessageTask tasks[1000];
    size_t counter;

    while (1)
    {
        if (len = msgrcv(msqid, (struct msgbuf*)&tasks[counter], sizeof(tasks[counter].info), TASK_MESSAGE, 0) < 0)
        {
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        pthread_create(&thid, (pthread_attr_t *)NULL, solution, &tasks[counter++]);
    }

    return 0;
}

void* solution (void* arg)
{
    struct MessageTask* task = (struct MessageTask*)arg;

    makeop(-1);

    struct MessageAnswer answer;
    answer.type = task->info.pid;
    answer.info.result = task->info.a * task->info.b;

    if (msgsnd(msqid, (struct msgbuf*)&answer, sizeof(answer.info), 0) < 0)
    {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
        exit(-1);
    }

    makeop(1);
}

void makeop (int n)
{
    struct sembuf mybuf;

    mybuf.sem_op  = n;
    mybuf.sem_flg = 0;
    mybuf.sem_num = 0;

    if (semop (semid, &mybuf, 1) < 0)
    {
        printf ("Can\'t wait for condition\n");
        exit (-1);
    }
}
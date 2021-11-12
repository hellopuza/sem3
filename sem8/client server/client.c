#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"

const size_t TASKS_NUM = 20;

int main ()
{
    int msqid;
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

    struct MessageTask msg_task;

    for (int i = 1; i <= TASKS_NUM; i++)
    {
        msg_task.type = TASK_MESSAGE;
        msg_task.info.a = i;
        msg_task.info.b = 10 - i;
        msg_task.info.pid = getpid();

        if (msgsnd(msqid, (struct msgbuf*)&msg_task, sizeof(msg_task.info), 0) < 0)
        {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
            exit(-1);
        }
        printf("client (%d): was sent %d %d %d\n", getpid(), msg_task.info.a, msg_task.info.b, msg_task.info.pid);
    }

    struct MessageAnswer msg_answ;

    for (int i = 1; i <= TASKS_NUM; i++)
    {
        if (len = msgrcv(msqid, (struct msgbuf*)&msg_answ, sizeof(msg_answ.info), getpid(), 0) < 0)
        {
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        printf("client (%d): was received %d\n", getpid(), msg_answ.info.result);
    }

    return 0;
}
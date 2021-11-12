#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"

size_t factorial(size_t num);

int main()
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

    struct Message msg;

    while (1)
    {
        if ( len = msgrcv(msqid, (struct msgbuf*)&msg, sizeof(msg.info), TASK_MESSAGE, 0) < 0)
        {
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        if (msg.type == LAST_MESSAGE)
        {
            msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
            exit(0);
        }

        printf("solver: was received %lu\n", msg.info.num);

        msg.type = ANSWER_MESSAGE;
        msg.info.num = factorial(msg.info.num);

        if (msgsnd(msqid, (struct msgbuf*)&msg, sizeof(msg.info), 0) < 0)
        {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
            exit(-1);
        }
        printf("solver: was sent %lu\n", msg.info.num);
    }

    return 0;
}

size_t factorial(size_t num)
{
    if (num < 2)
    {
        return 1;
    }
    else
    {
        return factorial(num - 1) * num;
    }
}
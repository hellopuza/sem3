#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"

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

    struct Message msg;

    for (int i = 1; i <= 10; i++)
    {
        msg.type = TASK_MESSAGE;
        msg.info.num = i;

        if (msgsnd(msqid, (struct msgbuf*)&msg, sizeof(msg.info), 0) < 0)
        {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
            exit(-1);
        }
        printf("sender: was sent %d\n", i);
    }

    msg.type = LAST_MESSAGE;
    if (msgsnd(msqid, (struct msgbuf*)&msg, 0, 0) < 0)
    {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
        exit(-1);
    }

    for (int i = 1; i <= 10; i++)
    {
        if (len = msgrcv(msqid, (struct msgbuf*)&msg, sizeof(msg.info), ANSWER_MESSAGE, 0) < 0)
        {
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        if (msg.type == LAST_MESSAGE)
        {
            msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
            exit(0);
        }

        printf("sender: was received %lu\n", msg.info.num);
    }

    return 0;
}
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"

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
        if ( len = msgrcv(msqid, (struct msgbuf*)&msg, sizeof(msg.info), 0, 0) < 0)
        {
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        if (msg.type == LAST_MESSAGE)
        {
            msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
            exit(0);
        }

        printf("message type = %ld, info = %d\n", msg.type, msg.info.num);
    }

    return 0;
}
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
    int i;

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

    for (i = 1; i <= 5; i++)
    {
        msg.type = 1;
        msg.info.num = i;

        if (msgsnd(msqid, (struct msgbuf*)&msg, sizeof(msg.info), 0) < 0)
        {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
            exit(-1);
        }
    }

    msg.type = LAST_MESSAGE;

    if (msgsnd(msqid, (struct msgbuf*)&msg, 0, 0) < 0)
    {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
        exit(-1);
    }

    return 0;
}
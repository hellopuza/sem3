#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

const size_t MAX_STR_LEN = 100000;

int main ()
{
    char* data;
    int shmid;
    int new = 1; /* Флаг необходимости инициализации элементов массива */
    char pathname[] = "file";
    key_t key;

    if ((key = ftok(pathname, 0)) < 0)
    {
        printf("Can\'t generate key\n");
        exit(-1);
    }

    if ((shmid = shmget(key, MAX_STR_LEN, 0666|IPC_CREAT|IPC_EXCL)) < 0)
    {
        if (errno != EEXIST)
        {
        printf("Can\'t create shared memory\n");
        exit(-1);
        }

        if ((shmid = shmget(key, MAX_STR_LEN, 0)) < 0)
        {
            printf("Can\'t find shared memory\n");
            exit(-1);
        }
        new = 0;
    }

    if ((data = (char*)shmat(shmid, NULL, 0)) == (char*)(-1))
    {
        printf("Can't attach shared memory\n");
        exit(-1);
    }

    FILE* fp = fopen("first.c", "r");
    fread(data, 1, MAX_STR_LEN, fp);
    fclose(fp);

    return 0;
} 
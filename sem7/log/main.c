#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

void* first (void* arg);
void* second (void* arg);

int semid;

int main (int argc, char* argv[])
{
    char pathname[] = "file";

    key_t key;
    if ((key = ftok(pathname, 0)) < 0)
    {
        printf("Can\'t generate key\n");
        exit(-1);
    }

    if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0)
    {
        printf("Can\'t get semid\n");
        exit(-1);
    }

    FILE* fp = fopen("txt.log", "wb");
    pthread_t thid1, thid2;
    int result;

    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = semid;
    buf.sem_op = 1;
    if (semop(semid, &buf, 1) < 0)
    {
        printf("Can\'t do semop\n");
        exit(-1);
    }

    result = pthread_create(&thid1, (pthread_attr_t *)NULL, first, fp);
    result = pthread_create(&thid2, (pthread_attr_t *)NULL, second, fp);

    pthread_join(thid1, NULL);
    pthread_join(thid2, NULL);
    fclose(fp);

    return 0;
}

void* first (void* arg)
{
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = semid;

    FILE* fp = (FILE*)arg;
    while (1)
    {
        buf.sem_op = -1;
        if (semop(semid, &buf, 1) < 0)
        {
            printf("Can\'t do semop\n");
            exit(-1);
        }

        fprintf(fp, "result = pthread_create(&thid1, (pthread_attr_t *)NULL, first, fp); ");

        buf.sem_op = 1;
        if (semop(semid, &buf, 1) < 0)
        {
            printf("Can\'t do semop\n");
            exit(-1);
        }
    }
}

void* second (void* arg)
{
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = semid;

    FILE* fp = (FILE*)arg;
    while (1)
    {
        buf.sem_op = -1;
        if (semop(semid, &buf, 1) < 0)
        {
            printf("Can\'t do semop\n");
            exit(-1);
        }

        fprintf(fp, "Эта программа получает доступ к одному системному семафору, ждет пока его значение не станет больше или равным 1 после запусков");

        buf.sem_op = 1;
        if (semop(semid, &buf, 1) < 0)
        {
            printf("Can\'t do semop\n");
            exit(-1);
        }
    }
}
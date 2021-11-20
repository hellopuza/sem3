#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DISHES_NUM   10
#define LAST_MESSAGE 255

#define ASSERT(cond, msg) \
        if (cond)         \
        {                 \
            printf(msg);  \
            exit(-1);     \
        }
        
const size_t TABLE_LIMIT = 5;

struct Dishes
{
    size_t washing_time;
    size_t drying_time;
} dishes[DISHES_NUM];

struct MessageToDryer
{
    long type;
};

int semid;
int msqid;

void fillDishes();
void TableOperation(int n);
void* Washer(void* arg);
void* Dryer(void* arg);

int main()
{
    fillDishes();

    key_t key;
    ASSERT(((key = ftok("file",0)) < 0), "ERROR: Can\'t generate key\n");
    ASSERT(((msqid = msgget(key, 0666 | IPC_CREAT)) < 0), "ERROR: Can\'t get msqid\n");
    ASSERT(((semid = semget (key, 1, 0666 | IPC_CREAT)) < 0), "ERROR: Can\'t get semid\n");

    TableOperation(TABLE_LIMIT);

    pthread_t washer_thid, dryer_thid;
    pthread_create(&washer_thid, NULL, Washer, NULL);
    pthread_create(&dryer_thid, NULL, Dryer, NULL);

    pthread_join(washer_thid, NULL);
    pthread_join(dryer_thid, NULL);

    return 0;
}

void fillDishes()
{
    FILE* file_washing = fopen("washing.txt", "r");
    FILE* file_drying  = fopen("drying.txt", "r");

    for (int i = 0; i < DISHES_NUM; i++)
    {
        size_t type;
        fscanf(file_washing, "%ld", &type);
        fscanf(file_washing, ":%ld", &dishes[type].washing_time);
        fscanf(file_drying,  "%ld", &type);
        fscanf(file_drying,  ":%ld", &dishes[type].drying_time);
    }

    fclose(file_washing);
    fclose(file_drying);
}

void TableOperation(int n)
{
    struct sembuf mybuf;

    mybuf.sem_op  = n;
    mybuf.sem_flg = 0;
    mybuf.sem_num = 0;

    ASSERT((semop (semid, &mybuf, 1) < 0), "ERROR: Can\'t wait for condition\n");
}

void* Washer(void* arg)
{
    FILE* file_dishes = fopen("dishes.txt", "r");

    size_t type;
    size_t num;
    while (fscanf(file_dishes, "%ld:%ld", &type, &num) != EOF)
    {
        for (size_t i = 0; i < num; i++)
        {
            printf("washer: started washing dishes %ld\n", type);
            sleep(dishes[type].washing_time);

            printf("washer: finished washing, put on the table\n");
            TableOperation(-1);

            struct MessageToDryer msg = {type};
            if (msgsnd(msqid, (struct msgbuf*)&msg, 0, 0) < 0)
            {
                printf("Can\'t send message to queue\n");
                msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
                exit(-1);
            }
        }
    }

    printf("washer: last dishes\n");

    struct MessageToDryer msg = {LAST_MESSAGE};
    if (msgsnd(msqid, (struct msgbuf*)&msg, 0, 0) < 0)
    {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *)NULL);
        exit(-1);
    }

    fclose(file_dishes);
}

void* Dryer(void* arg)
{
    struct MessageToDryer msg;
    while (1)
    {
        if (msgrcv(msqid, (struct msgbuf*)&msg, 0, 0, 0) < 0)
        {
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        if (msg.type == LAST_MESSAGE)
        {
            printf("dryer: last dishes\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
            break;
        }

        TableOperation(1);
        printf("dryer: started drying dishes %ld\n", msg.type);
        sleep(dishes[msg.type].drying_time);

        printf("dryer: finished drying\n");
    }
}
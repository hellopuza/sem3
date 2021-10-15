#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define ASSERT(cond, msg) \
        if (cond)         \
        {                 \
            printf(msg);  \
            exit(-1);     \
        }

const char* const FIFO_NAME1 = "12";
const char* const FIFO_NAME2 = "21";

const size_t MAX_STRLEN = 10000;

void* print_mes (void *fifo);
void* read_mes  (void *fifo);

int main (int argc, char* argv[])
{
    ASSERT((argc < 2), "number of client required\n");

    int num = atoi(argv[1]);
    ASSERT((num != 1) && (num != 2), "wrong number of client\n");

    umask(0);
    
    if (mknod(FIFO_NAME1, S_IFIFO | 0666, 0) == -1)
    {
        ASSERT((errno != EEXIST), "Can't create fifo\n");
    }

    if (mknod(FIFO_NAME2, S_IFIFO | 0666, 0) == -1)
    {
        ASSERT((errno != EEXIST), "Can't create fifo\n");
    }

    char* fifo1;
    char* fifo2;
    if (atoi(argv[1]) == 1)
    {
        fifo1 = FIFO_NAME1;
        fifo2 = FIFO_NAME2;
    }
    else
    {
        fifo1 = FIFO_NAME2;
        fifo2 = FIFO_NAME1;
    }

    pthread_t thid1, thid2;
    int result;

    result = pthread_create(&thid1, (pthread_attr_t *)NULL, print_mes, fifo1);
    ASSERT((result != 0), "Error on thread create\n");

    result = pthread_create(&thid2, (pthread_attr_t *)NULL, read_mes, fifo2);
    ASSERT((result != 0), "Error on thread create\n");


    pthread_join(thid1, (void **)NULL);
    pthread_join(thid2, (void **)NULL);

    return 0;
}

void* print_mes (void *fifo)
{
    int fd = open((char*)fifo, O_RDONLY);
    char str[MAX_STRLEN];
    while (1)
    {
        read(fd, str, MAX_STRLEN);
        printf(str);
    }

    return NULL;
}

void* read_mes (void *fifo)
{
    int fd = open((char*)fifo, O_WRONLY);
    char str[MAX_STRLEN];
    while (1)
    {
        fgets(str, MAX_STRLEN, stdin);
        write(fd, str, MAX_STRLEN);
    }

    return NULL;
}
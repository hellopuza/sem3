#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

const char* const FIFO_NAME1 = "12";
const char* const FIFO_NAME2 = "21";

const size_t MAX_STRLEN = 1000;

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("number of client required\n");
        exit(-1);
    }

    char* fifo1;
    char* fifo2;
    if (atoi(argv[1]) == 1)
    {
        fifo1 = FIFO_NAME1;
        fifo2 = FIFO_NAME2;
    }
    else if (atoi(argv[1]) == 2)
    {
        fifo1 = FIFO_NAME2;
        fifo2 = FIFO_NAME1;
    }
    else
    {
        printf("wrong number of client\n");
        exit(-1);
    }

    umask(0);
    
    if (mknod(FIFO_NAME1, S_IFIFO | 0666, 0) == -1)
    {
        if (errno != EEXIST)
        {
            printf("Can't create fifo\n");
            exit(-1);
        }
    }

    if (mknod(FIFO_NAME2, S_IFIFO | 0666, 0) == -1)
    {
        if (errno != EEXIST)
        {
            printf("Can't create fifo\n");
            exit(-1);
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        int fd1 = open(fifo1, O_RDONLY);
        char str[MAX_STRLEN];
        while (1)
        {
            read(fd1, str, MAX_STRLEN);
            printf(str);
        }
    }
    else
    {
        int fd2 = open(fifo2, O_WRONLY);
        char str[MAX_STRLEN];
        while (1)
        {
            fgets(str, MAX_STRLEN, stdin);
            write(fd2, str, MAX_STRLEN);
        }
    }

    return 0;
}
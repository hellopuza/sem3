#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void createProcess (int num);

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("number of processes required\n");
        return 0;
    }

    int num = atoi(argv[1]);
    if (num == 0)
    {
        printf("incorrect input\n");
        return 0;
    }
        
    createProcess (num);

    return 0;
}

void createProcess (int num)
{
    if (num > 0)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            createProcess(num - 1);
            exit(0);
        }

        printf("(%d) started: %d\n", num, pid);
        int status = 0;
        wait(&status);
        printf("(%d) finished: %d\n", num, pid);
    }
}
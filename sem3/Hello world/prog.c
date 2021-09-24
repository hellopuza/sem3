#include <stdio.h>
#include <unistd.h>

int main()
{
    int pid = fork();
    if (pid == 0)
    {
        execlp("gcc", "gcc", "main.c", "-o", "hello", NULL);
    }
    else
    {
        int source = 0;
        wait(&source);
        execlp("./hello", "./hello", NULL);
    }
    return 0;
}
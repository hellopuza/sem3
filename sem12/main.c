#include <signal.h>
#include <stdio.h>
#include <unistd.h>

pid_t child_pid;

void handler1(int arg)
{
	printf("RT\n");
	kill(child_pid, SIGUSR2);
}

void handler2(int arg)
{
	printf("champion!\n");
	kill(getppid(), SIGUSR1);
}

int main()
{
    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);

    pid_t pid = fork();
    if (pid == 0)
    {
        while(1);
    }
    else
    {
        child_pid = pid;
        kill(child_pid, SIGUSR2);
		while(1);
    }

    return 0;
}
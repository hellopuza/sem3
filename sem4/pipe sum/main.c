#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char* argv[])
{
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);

    int pid = fork();
    if (pid == 0)
    {
        int ab[] = {1, 2};
        write(fd1[1], ab, sizeof(ab));
        int res = 0;
        read(fd2[0], &res, sizeof(res));
        printf("%d\n", res);
    }
    else
    {
        int ab[2];
        read(fd1[0], ab, sizeof(ab));
        int res = ab[0] + ab[1];
        write(fd2[1], &res, sizeof(res));
    }

    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    
    return 0;
}
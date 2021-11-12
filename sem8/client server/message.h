#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <unistd.h>

enum Messages
{
    TASK_MESSAGE = 1,
};

struct MessageTask
{
    long type;
    struct
    {
        int a;
        int b;
        pid_t pid;
    } info;
};

struct MessageAnswer
{
    long type;
    struct
    {
        int result;
    } info;
};
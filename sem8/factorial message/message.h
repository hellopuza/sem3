#include <sys/ipc.h>
#include <sys/msg.h>

enum Messages
{
    TASK_MESSAGE = 1,
    ANSWER_MESSAGE = 2,
    LAST_MESSAGE = 255,
};

struct Message
{
    long type;
    struct
    {
        size_t num;
    } info;
};
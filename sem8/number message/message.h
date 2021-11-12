#include <sys/ipc.h>
#include <sys/msg.h>

#define LAST_MESSAGE 255

struct Message
{
    long type;
    struct
    {
        int num;
    } info;
};
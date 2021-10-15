#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "Split.h"

#define ASSERT(cond, msg) \
        if (cond)         \
        {                 \
            printf(msg);  \
            exit(-1);     \
        }

const size_t MAX_STR_LEN = 10000;
const size_t TIMEOUT     = 5;
char const * const DELIMETERS = " \t\n";

struct line
{
    char* str;
    char** tokens;
    size_t tokens_num;
    size_t time;
    pid_t  pid;
};

struct line* getLines    (FILE* fp, size_t lines_num);
void         deleteLines (struct line* lines, size_t lines_num);
int          compareTime (void* line1, void* line2);

int main (int argc, char* argv[])
{
    ASSERT((argc < 2), "file name required\n");

    FILE* fp = fopen(argv[1], "r");
    ASSERT((fp == NULL), "file not found\n");

    size_t lines_num = 0;
    char first_line[MAX_STR_LEN];
    fgets(first_line, MAX_STR_LEN, fp);
    sscanf(first_line, "%lu", &lines_num);

    struct line* lines = getLines(fp, lines_num);
    fclose(fp);

    qsort(lines, lines_num, sizeof(struct line), compareTime);

    for (size_t i = 0; i < lines_num; i++)
    {
        pid_t pid1 = fork();
        if (pid1 == 0)
        {
            pid_t pid2 = fork();
            if (pid2 == 0)
            {
                sleep(lines[i].time + TIMEOUT);
                kill(getppid(), SIGINT);
                return 0;
            }
            else
            {
                sleep(lines[i].time);
                execvp(lines[i].tokens[1], lines[i].tokens + 1);
                printf("wrong command: %s\n", lines[i].tokens[1]);
                exit(-1);
            }
        }
        lines[i].pid = pid1;
    }

    for (size_t i = 0; i < lines_num; i++)
    {
        int status;
        waitpid(lines[i].pid, &status, WUNTRACED);
        printf("process %d, completion status: %d\n", lines[i].pid, status);
    }

    deleteLines(lines, lines_num);

    return 0;
}

struct line* getLines (FILE* fp, size_t lines_num)
{
    struct line* lines = (struct line*)calloc(lines_num + 1, sizeof(struct line)); 

    for (size_t i = 0; i < lines_num; i++)
    {
        lines[i].str    = (char*)calloc(MAX_STR_LEN, 1);
        lines[i].tokens = (char**)calloc(MAX_STR_LEN, sizeof(char*));

        fgets(lines[i].str, MAX_STR_LEN, fp);
        Split(lines[i].str, DELIMETERS, lines[i].tokens, &lines[i].tokens_num);

        lines[i].time = atoi(lines[i].tokens[0]);
    }

    return lines;
}

void deleteLines (struct line* lines, size_t lines_num)
{
    for (size_t i = 0; i < lines_num; i++)
    {
        free((void*)lines[i].str);
        free((void*)lines[i].tokens);
    }

    free(lines);
}

int compareTime (void* line1, void* line2)
{
    return ((struct line*)line1)->time - ((struct line*)line2)->time;
}

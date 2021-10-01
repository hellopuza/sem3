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

    clock_t prog_start = clock();

    for (size_t i = 0; i < lines_num; i++)
    {
        while ((clock() - prog_start) / CLOCKS_PER_SEC < lines[i].time);

        pid_t pid1 = fork();
        if (pid1 == 0)
        {
            int fd[2];
            pipe(fd);

            pid_t pid2 = fork();
            if (pid2 == 0)
            {
                pid_t pid3 = fork();
                if (pid3 == 0)
                {
                    pid_t kill_pid = getpid();
                    write(fd[1], &kill_pid, sizeof(kill_pid));

                    sleep(TIMEOUT);
                    printf("kill: %d\n", getppid());
                    kill(getppid(), SIGINT);
                    return 0;
                }
                else
                {
                    execvp(lines[i].tokens[1], lines[i].tokens + 1);
                }
            }
            else
            {
                pid_t kill_pid = 0;
                read(fd[0], &kill_pid, sizeof(kill_pid));

                int status = 0;
                wait(&status);
                kill(kill_pid, SIGINT);
                return 0;
            }
        }
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

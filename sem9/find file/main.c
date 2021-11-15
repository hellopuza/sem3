#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>

const size_t MAX_PATH_LEN = 1000;
const size_t MAX_PATH_NUM = 1000;

void findFile(char* filename, char* current_dir, int depth, int* count, char** paths);

int main(int argc, char* argv[])
{
    if (argc == 4)
    {
        char* current_dir = argv[1];
        char* filename = argv[3];
        int depth = atoi(argv[2]);

        int count = 0;
        char** paths = (char**)calloc(MAX_PATH_NUM, sizeof(char*));

        findFile(filename, current_dir, depth, &count, paths);
        if (count)
        {
            printf("found %d files in next directories:\n", count);
            for (size_t i = 0; i < count; i++)
            {
                printf("%s\n", paths[i]);
                free(paths[i]);
            }
        }
        else
        {
            printf("file not found\n");
        }
        free(paths);
    }
    else
    {
        printf("ERROR: directory, depth and filename required\n");
        exit(-1);
    }
    return 0;
}

void findFile(char* filename, char* current_dir, int depth, int* count, char** paths)
{
    if (depth > 0)
    {
        DIR* pDir = opendir(current_dir);
        struct dirent* pDirent;
        while (pDirent = readdir(pDir))
        {
            if ((strcmp(pDirent->d_name, "..") != 0) && (strcmp(pDirent->d_name, ".") != 0))
            {
                char* new_dir = (char*)calloc(MAX_PATH_LEN, 1);
                strcpy(new_dir, current_dir);
                strcat(new_dir, "/");
                strcat(new_dir, pDirent->d_name);

                DIR* check_dir = opendir(new_dir);
                if (check_dir != NULL)
                {
                    closedir(check_dir);
                    findFile(filename, new_dir, depth - 1, count, paths);
                    free(new_dir);
                }
                else
                {
                    if (strcmp(filename, pDirent->d_name) == 0)
                    {
                        paths[*count] = new_dir;
                        (*count)++;
                    }
                }
            }
        }
        closedir(pDir);
    }
}
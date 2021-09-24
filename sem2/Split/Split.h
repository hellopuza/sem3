#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Split (char* string, char* delimeters, char** tokens, size_t* tokens_num)
{
    *tokens_num = 0;
    string = strtok(string, delimeters);

    while (string != NULL)
    {
        tokens[*tokens_num] = string;
        (*tokens_num)++;

        string = strtok(NULL, delimeters);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_STR_LEN = 1000; 

void Split (char* string, char* delimeters, char** tokens, int* tokens_num)
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

int main()
{
    char* str = (char*)calloc(MAX_STR_LEN, 1);
    gets(str);

    char* delimeters = " \t";
    int tokens_num = 0;

    char** tokens = (char**)calloc(MAX_STR_LEN, 1);

    Split(str, delimeters, tokens, &tokens_num);
    
    for (int i = 0; i < tokens_num; i++)
    {
        printf("%s\n", tokens[i]);
    }

    free(str);
    free(tokens);

    return 0;
}
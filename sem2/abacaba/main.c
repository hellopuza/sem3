#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void GenerateString (int n, char* str)
{
    *str = 'a';

    size_t len = 1;

    for (int i = 0; i < n; i++)
    {
        str[len] = i + 'a' + 1;
        strncpy(str + len + 1, str, len);
        len = len * 2 + 1;
    }

    str[len] = '\0';
}

int main()
{
    int n = 0;
    scanf("%d", &n);
    char* str = (char*)malloc(1 << (n + 1));

    GenerateString(n, str);
    printf("%s\n", str);

    free(str);
    return 0;
}
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

enum NetworkEnum
{
    SERVER,
    CLIENT,
};

struct Network
{
    struct sockaddr_in servaddr, cliaddr;
    int sockfd;
    int type;
};

const size_t PORT = 55555;

const size_t GRID_WIDTH  = 7;
const size_t GRID_HEIGHT = 6;

const size_t BOARD_WIDTH  = (GRID_WIDTH + 1) * 2;
const size_t BOARD_HEIGHT = GRID_HEIGHT + 1;

const char WALL     = '|';
const char NO_DISK  = ' ';
const char DISK_ONE = 'O';
const char DISK_TWO = 'X';

void makeClient(struct Network* client, char* ip_addr);
void makeServer(struct Network* server);
void connectPlayer(struct Network* network);

void runGame(struct Network* network);

size_t scanColumn();
char* makeBoard();
char getDisk(char* board, size_t column, size_t row);
void setDisk(char* board, size_t column, size_t row, char disk);
int DropDisk(char* board, size_t column, char disk);
int checkFour(char* board);
int checkFullness(char* board);
int updateBoard(char* board, char disk);

int main(int argc, char *argv[])
{
    struct Network network;

    if (argc == 1)
    {
        makeServer(&network);
    }
    else
    {
        makeClient(&network, argv[1]);
    }
    connectPlayer(&network);

    runGame(&network);

    return 0;
}

void makeServer(struct Network* server)
{
    bzero(&server->servaddr, sizeof(server->servaddr));
    server->servaddr.sin_family = AF_INET;
    server->servaddr.sin_port = htons(PORT);
    server->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((server->sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror(NULL);
        exit(1);
    }
    if (bind(server->sockfd, (struct sockaddr*)&server->servaddr, sizeof(server->servaddr)) < 0)
    {
        perror(NULL);
        close(server->sockfd);
        exit(1);
    }
    server->type = SERVER;
}

void makeClient(struct Network* client, char* ip_addr)
{
    if ((client->sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror(NULL);
        exit(1);
    }

    bzero(&client->cliaddr, sizeof(client->cliaddr));
    client->cliaddr.sin_family = AF_INET;
    client->cliaddr.sin_port = htons(0);
    client->cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(client->sockfd, (struct sockaddr*)&client->cliaddr, sizeof(client->cliaddr)) < 0)
    {
        perror(NULL);
        close(client->sockfd);
        exit(1);
    }

    bzero(&client->servaddr, sizeof(client->servaddr));
    client->servaddr.sin_family = AF_INET;
    client->servaddr.sin_port = htons(PORT);
    if (inet_aton(ip_addr, &client->servaddr.sin_addr) == 0)
    {
        printf("Invalid IP address\n");
        close(client->sockfd);
        exit(1);
    }
    client->type = CLIENT;
}

void connectPlayer(struct Network* network)
{
    switch (network->type)
    {
    case SERVER:
    {
        printf("Waiting for other player...\n");

        int num;
        socklen_t clilen = sizeof(network->cliaddr);
        if (recvfrom(network->sockfd, &num, sizeof(num), 0, (struct sockaddr*)&network->cliaddr, &clilen) < 0)
        {
            perror(NULL);
            close(network->sockfd);
            exit(1);
        }
        if (sendto(network->sockfd, &num, sizeof(num), 0, (struct sockaddr*)&network->cliaddr, sizeof(network->cliaddr)) < 0)
        {
            perror(NULL);
            close(network->sockfd);
            exit(1);
        }
        break;
    }
    case CLIENT:
    {
        int num = 1;
        printf("Wrong ip address\n");
        if (sendto(network->sockfd, &num, sizeof(num), 0, (struct sockaddr *)&network->servaddr, sizeof(network->servaddr)) < 0)
        {
            perror(NULL);
            close(network->sockfd);
            exit(1);
        }
        if (recvfrom(network->sockfd, &num, sizeof(num), 0, (struct sockaddr*)NULL, NULL) < 0)
        {
            perror(NULL);
            close(network->sockfd);
            exit(1);
        }
        break;
    }
    }
}

void runGame(struct Network* network)
{
    #define ERROR_EXIT          \
        perror(NULL);           \
        close(network->sockfd); \
        free(board);            \
        return;
    
    system("clear");

    char* board = makeBoard();
    printf("%s", board);

    while (1)
    {
        switch (network->type)
        {
        case SERVER:
        {
            printf("Current disk %c\n", DISK_ONE);
            size_t input_column = scanColumn();
            while (DropDisk(board, input_column, DISK_ONE))
            {
                printf("Wrong movement!\n");
                input_column = scanColumn();
            }

            if (sendto(network->sockfd, &input_column, sizeof(input_column), 0, (struct sockaddr*)&network->cliaddr, sizeof(network->cliaddr)) < 0)
            {
                ERROR_EXIT;
            }

            if (updateBoard(board, DISK_ONE)) return;

            if (recvfrom(network->sockfd, &input_column, sizeof(input_column), 0, (struct sockaddr*)NULL, NULL) < 0)
            {
                ERROR_EXIT;
            }

            DropDisk(board, input_column, DISK_TWO);
            if (updateBoard(board, DISK_TWO)) return;

            break;
        }
        case CLIENT:
        {
            size_t input_column;
            if (recvfrom(network->sockfd, &input_column, sizeof(input_column), 0, (struct sockaddr*)NULL, NULL) < 0)
            {
                ERROR_EXIT;
            }

            DropDisk(board, input_column, DISK_ONE);
            if (updateBoard(board, DISK_ONE)) return;

            printf("Current disk %c\n", DISK_TWO);
            input_column = scanColumn();
            while (DropDisk(board, input_column, DISK_TWO))
            {
                printf("Wrong movement!\n");
                input_column = scanColumn();
            }

            if (sendto(network->sockfd, &input_column, sizeof(input_column), 0, (struct sockaddr *)&network->servaddr, sizeof(network->servaddr)) < 0)
            {
                ERROR_EXIT;
            }

            if (updateBoard(board, DISK_TWO)) return;

            break;
        }
        }
    }

    #undef ERROR_EXIT
}

size_t scanColumn()
{
    const size_t MAX_STR_LEN = 8;

    size_t num = 0;
    char str[MAX_STR_LEN];
    char* endstr = (char*)"";

    printf("Enter column: ");

    char* err = fgets(str, MAX_STR_LEN, stdin);
    num = strtoul(str, &endstr, 10);

    while ((endstr[0] != '\n') || !err)
    {
        printf("Try again: ");
        err = fgets(str, MAX_STR_LEN, stdin);
        num = strtoul(str, &endstr, 10);
    }

    return num;
}

char* makeBoard()
{
    char* board = (char*)calloc(BOARD_WIDTH * BOARD_HEIGHT + 2, 1);
    for (size_t y = 0; y < GRID_HEIGHT; y++)
    {
        for (size_t x = 0; x < BOARD_WIDTH - 1; x++)
        {
            board[y * BOARD_WIDTH + x] = (x % 2) ? NO_DISK : WALL;
        }
        board[y * BOARD_WIDTH + BOARD_WIDTH - 1] = '\n';
    }

    for (size_t x = 0; x < BOARD_WIDTH - 1; x++)
    {
        board[GRID_HEIGHT * BOARD_WIDTH + x] = (x % 2) ? (((x + 1) / 2) % 10 + '0') : WALL;
    }
    board[GRID_HEIGHT * BOARD_WIDTH + BOARD_WIDTH - 1] = '\n';

    return board;
}

char getDisk(char* board, size_t column, size_t row)
{
    return board[(GRID_HEIGHT - row) * BOARD_WIDTH + column * 2 - 1];
}

void setDisk(char* board, size_t column, size_t row, char disk)
{
    board[(GRID_HEIGHT - row) * BOARD_WIDTH + column * 2 - 1] = disk;
}

int DropDisk(char* board, size_t column, char disk)
{
    if ((column < 1) || (column > GRID_WIDTH))
        return 1;

    for (size_t row = 1; row <= GRID_HEIGHT; row++)
    {
        if (getDisk(board, column, row) == NO_DISK)
        {
            setDisk(board, column, row, disk);
            return 0;
        }
    }
    return 1;
}

int checkFour(char* board)
{
    for (size_t row = 1; row <= GRID_HEIGHT; row++)
    {
        for (size_t column = 1; column <= GRID_WIDTH; column++)
        {
            char disk = getDisk(board, column, row);
            if (disk == NO_DISK)
                break;

            if (column <= GRID_WIDTH - 3)
            {
                if (disk == getDisk(board, column + 1, row) &&
                    disk == getDisk(board, column + 2, row) &&
                    disk == getDisk(board, column + 3, row))
                    {
                        return 1;
                    }
            }

            if (row <= GRID_HEIGHT - 3)
            {
                if (disk == getDisk(board, column, row + 1) &&
                    disk == getDisk(board, column, row + 2) &&
                    disk == getDisk(board, column, row + 3))
                    {
                        return 1;
                    }
            }

            if ((column <= GRID_WIDTH - 3) && (row <= GRID_HEIGHT - 3))
            {
                if (disk == getDisk(board, column + 1, row + 1) &&
                    disk == getDisk(board, column + 2, row + 2) &&
                    disk == getDisk(board, column + 3, row + 3))
                    {
                        return 1;
                    }
            }

            if ((column >= 4) && (row <= GRID_HEIGHT - 3))
            {
                if (disk == getDisk(board, column - 1, row + 1) &&
                    disk == getDisk(board, column - 2, row + 2) &&
                    disk == getDisk(board, column - 3, row + 3))
                    {
                        return 1;
                    }
            }
        }
    }

    return 0;
}

int checkFullness(char* board)
{
    for (size_t row = 1; row <= GRID_HEIGHT; row++)
    {
        for (size_t column = 1; column <= GRID_WIDTH; column++)
        {
            if (getDisk(board, column, row) == NO_DISK)
            {
                return 0;
            }
        }
    }
    return 1;
}

int updateBoard(char* board, char disk)
{
    system("clear");
    printf("%s", board);

    if (checkFour(board))
    {
        printf("Winner %c\n", disk);
        free(board);
        return 1;
    }
    if (checkFullness(board))
    {
        printf("Draw\n");
        free(board);
        return 1;
    }

    return 0;
}
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

struct Sockets
{
    int sockfd, newsockfd;
};

void* processing(void* arg);

void main()
{
    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in servaddr, cliaddr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror(NULL);
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family= AF_INET;
    servaddr.sin_port= htons(51000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr *) &servaddr, 
    sizeof(servaddr)) < 0)
    {
        perror(NULL);
        close(sockfd);
        exit(1);
    }

    if(listen(sockfd, 5) < 0)
    {
        perror(NULL);
        close(sockfd);
        exit(1);
    }

    while(1)
    {
        clilen = sizeof(cliaddr);
        if((newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen)) < 0)
        {
            perror(NULL);
            close(sockfd);
            exit(1);
        }
        struct Sockets sock = {sockfd, newsockfd};
        pthread_t thid;
        if (pthread_create(&thid, (pthread_attr_t *)NULL, processing, &sock) != 0)
        {
            printf("Error on thread create\n");
            exit(1);
        }
    }
}

void* processing(void* arg)
{
    struct Sockets sock = *(struct Sockets*)arg;
    int n;
    char line[1000];

    while((n = read(sock.newsockfd, line, 999)) > 0)
    {
        if((n = write(sock.newsockfd, line, strlen(line)+1)) < 0)
        {
            perror(NULL);
            close(sock.sockfd);
            close(sock.newsockfd);
            exit(1);
        }
    }

    if(n < 0)
    {
        perror(NULL);
        close(sock.sockfd);
        close(sock.newsockfd);
        exit(1);
    }
    close(sock.newsockfd);

    return NULL;
}
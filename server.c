#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 36079
#define MAX_CLIENTS 5

struct arg {
    int which;
    int * nb_clients;
    int * clients;
};

struct arg_cli {
    int * stop;
    int * nb_clients;
    int * clients;
    int * serverfd;
};

void * handle_client(void * varg)
{
    struct arg * args = varg;
    int clifd = args->clients[args->which];
    while (1) {
        if (args->clients[args->which] == -1)
            break;
        sleep(0.5);
        char buffer [1024] = {0};
        int valread = read(clifd,buffer,1024);
        if (valread > 0) {
            for(int i = 0; i < *(args->nb_clients); i++)
            {
                if (args->clients[i] != -1)
                    send(args->clients[i], buffer, strlen(buffer), 0);
            }
        }
        else if (send(clifd, "\00\00\00", 3, MSG_NOSIGNAL) < 0)
        {
            close(clifd);
            args->clients[args->which] = -1;
            break;
        }
    }
    printf("client %d disconnected\n", args->which);
    free(args);
    return NULL;
}

void * handle_cli(void * varg)
{
    struct arg_cli * args = varg;
    while (1) {
        char answer[256];
        scanf("%s", answer);
        if (strncmp(answer,"exit",4) == 0) {
            for (int i = 0; i < *(args->nb_clients); i++)
            {
                if (args->clients[i] != -1) {
                    puts("debug");
                    close(args->clients[i]);
                    args->clients[i] = -1;
                }
            }
            *(args->stop) = 1;
            shutdown(*(args->serverfd),SHUT_RDWR);
            *(args->serverfd) = -1;
            break;
        }
    }
    return NULL;
}

int main(void) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    int addrlen = sizeof(addr);
    
    if (bind(sockfd, (struct sockaddr*) &addr, addrlen) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Listening on port 0.0.0.0:%d\n", PORT);
    int sockfd_client, valread;
    int clients[MAX_CLIENTS];
    pthread_t tids[MAX_CLIENTS];
    int nb_clients = 0;
    struct arg_cli args_cli;
    int stop = 0;
    args_cli.stop = &stop;
    args_cli.clients = clients;
    args_cli.nb_clients = &nb_clients;
    args_cli.serverfd = &sockfd;
    pthread_t cli_tid;
    pthread_create(&cli_tid, NULL, handle_cli, &args_cli);
    while (!stop || nb_clients < MAX_CLIENTS) 
    {
        if ((sockfd_client = accept(sockfd, (struct sockaddr*)&addr, (socklen_t*) &addrlen)) < 0)
        {
            if (sockfd == -1)
                break;
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        else {
            clients[nb_clients] = sockfd_client;
            nb_clients++;
            struct arg * args = malloc(sizeof(struct arg));
            args->which = nb_clients - 1;
            args->clients = clients;
            args->nb_clients = &nb_clients;
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, args);
            tids[nb_clients-1] = tid;
        }
    }
    printf("Goodbye\n");
    for (int i = 0; i < nb_clients; i++)
    {
        pthread_join(tids[i], NULL);
    }
    pthread_join(cli_tid,NULL);
    if (sockfd != -1)
        shutdown(sockfd, SHUT_RDWR);
    return 0;
}


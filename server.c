#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 36078
#define MAX_CLIENTS 5

struct arg {
    int which;
    int * nb_clients;
    int * clients;
};

void * handle_client(void * varg)
{
    struct arg * args = varg;
    int clifd = args->clients[args->which];
    while (1) {
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
            break;
        }
    }
    close(clifd);
    printf("client %d disconnected\n", args->which);
    args->clients[args->which] = -1;
    free(args);
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
    while (nb_clients < MAX_CLIENTS) 
    {
        if ((sockfd_client = accept(sockfd, (struct sockaddr*)&addr, (socklen_t*) &addrlen)) < 0)
        {
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
    for (int i = 0; i < nb_clients; i++)
    {
        pthread_join(tids[i], NULL);
    }
    printf("Goodbye\n");
    shutdown(sockfd, SHUT_RDWR);
    return 0;
}


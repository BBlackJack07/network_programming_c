#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 36079

#ifdef __APPLE__
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

void * receive_msg(void * vargs)
{
    int * sockfd_p = vargs;
    int sockfd = *sockfd_p;
    char buffer[1024] = {0};
    int valread;
    while(1) {
      sleep(0.5);
      valread = read(sockfd, buffer, 1024);
      if (valread > 0) {
          printf("\n%s\n", buffer);
      }
      else if (send(sockfd, "\00\00\00", 3, MSG_NOSIGNAL) < 0)
          break;
    }
    return NULL;
}

int main(void)
{
    int sockfd;
    char * hello = "hello, world";
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
   
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
    {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;
    pthread_create(&tid,NULL, receive_msg, &sockfd);
    char buffer[1024];
    int i = 0;
    while (strncmp(buffer,"/exit",5) != 0) {
        printf(">>> ");
        char c;
        while ((c = getc(stdin)) != '\n' && i < 1024) 
        {
            if (c == EOF)
                break;
            buffer[i] = c;
            i++;
        }
        if (c == EOF)
            break;
        send(sockfd, buffer, i, 0);
        i = 0;
        sleep(0.5);
    }
    
    close(sockfd);
    pthread_join(tid, NULL);
    return 0;
}

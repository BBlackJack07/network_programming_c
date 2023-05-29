#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 36078

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

   send(sockfd, hello, strlen(hello), 0);
   char buffer[1024] = {0};
   int valread;
   while(1) {
      sleep(0.5);
      valread = read(sockfd, buffer, 1024);
      printf("> %s\n", buffer);
   }
   close(sockfd);
   return 0;
}

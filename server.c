#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#define port 8080
#define size 1024

void error(char *mesg)
{
    perror(mesg);
    exit(1);
}

int main()
{
    int sockfd,clientfd;
    char buffer[size];
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    char *response="HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/plain\r\n"
                   "\r\n"
                   "Hello Akhil";
    if(sockfd<0)
    {
        error("Problem in server socket creation\n");
    }
    struct sockaddr_in server,client;
    socklen_t l=sizeof(client);
    server.sin_family=AF_INET;
    server.sin_port=htons(port);
    server.sin_addr.s_addr=INADDR_ANY;
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if(bind(sockfd,(struct sockaddr*)&server,sizeof(server))<0)
    {
        error("Error in binding\n");
    }
    printf("Listening on port %d\n",port);
    if(listen(sockfd,5)<0)
    {
      error("Listen failed");
    }
    while(1)
    {
        clientfd=accept(sockfd,(struct sockaddr*)&client,&l);
        if(clientfd<0)
        {
          error("Error in client socket creation\n");
          continue;
        }
        printf("client connected\n");
        memset(buffer,0,size);
        int bytes=recv(clientfd,buffer,size-1,0);
        if(bytes > 0) 
        {
            buffer[bytes] = '\0';  
            char *s = strtok(buffer, " ");
            char *s1 = strtok(NULL, " ");
            if(s1 != NULL) 
            {
                printf("Requested Path: %s\n", s1);
            }
            send(clientfd, response, strlen(response), 0);
        }
        close(clientfd);
    }
    close(sockfd);
    return 0;
}

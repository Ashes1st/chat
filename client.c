 //
//  client
// 
//  Created by Ashe_s_ on 03.12.2017.
//  Copyright © 2017 ashes. All rights reserved.
//

/* client.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h> 
#include <sys/wait.h> 
#include <signal.h>

#define BUFFERSIZE 128

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int create_socket(int sockfd, char IP[], char PORT[])
{
    struct addrinfo hints, *servinfo, *p;
    int rv;

    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    
    for( p = servinfo; p != NULL; p = p->ai_next )
    {
        if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 )
        {
            perror("client: socket\n");
            continue;
        }                                                   

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect\n");
            continue;
        }

        break;
    }

    if(p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 1;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);

    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

    return sockfd;
}

void get_history(int sockfd, char *msg, char *buffer)
{
    char doc_true[] = "+++++++++++++++";
    char doc_false[] = "-";

    write(sockfd, msg, strlen(msg));
    read(sockfd, buffer, BUFFERSIZE);
    
    int num = atoi(buffer);
    int k = 0;

    printf("\nvvvv   history    vvvv\n");

    while(k < num)
    {
        memset(buffer, 0, BUFFERSIZE);
        int t = read(sockfd, buffer, BUFFERSIZE);
        
        if(t == BUFFERSIZE)
        {
            write(sockfd, doc_true, strlen(doc_true));
            printf("%s", buffer);
            k++;
        }else
        {
            write(sockfd, doc_false, strlen(doc_false));
        }
    }
    
    printf("\n^^^^   history    ^^^^\n\n");
}

int main(int argc, char *argv[])
{

    fd_set master;                          // главный список дескрипторов
    fd_set use_fds;                             // временный список дескрипторов для селект
    int fdmax;                                  // максимальный номер дескриптора

    FD_ZERO(&master);
    FD_ZERO(&use_fds);

    int sockfd, numbytes;
    
    char buffer[BUFFERSIZE];

    if (argc != 4) 
    {
        fprintf(stderr,"usage: nickname ip port\n");
        return 1; 
    }

    char *dv = ": ";
    
    char t[BUFFERSIZE];
    memset(t, 0, BUFFERSIZE);

    char *nickname = strcat(t, argv[1]);
    nickname = strcat(nickname, dv);
    int nick_len = (int)strlen(nickname);

    if((sockfd = create_socket(sockfd, argv[2], argv[3])) < 0)
    {
        printf("Error: create socket");
        return 1;
    }

    FD_ZERO(&master);
    FD_SET(sockfd, &master);
    FD_SET(0, &master);//stdin

    fdmax = sockfd + 1;

    int Run = 1;


    while(Run) 
    {
        use_fds = master;
        select(fdmax, &use_fds, NULL,NULL,NULL);
        
        for(int sock = 0; sock < fdmax; sock++)
          	if(FD_ISSET(sock, &use_fds))
           {
               if(sock == sockfd)
               {   /*Accept data from open socket */

                   memset(buffer, 0, BUFFERSIZE);
                   int result = read(sockfd, buffer, BUFFERSIZE);
                buffer[result] = '\0';
                printf("%s", buffer);

               } else if(sock == 0)
               {
                   memset(buffer, 0, sizeof buffer);
                   memset(nickname + nick_len, 0, (sizeof nickname) - nick_len);
                   fgets(buffer, BUFFERSIZE, stdin);

                   char *msg = strcat(nickname, buffer);

                   if(strcmp(buffer, "\n"))
                   {
                       if (strcmp(buffer, "quit\n") == 0) 
                       {

                           send(sock, msg, strlen(msg), 0);
                           printf("You're disconnect.\n");
                           close(sock); //close the current client
                           Run = 0; //end program

                       } else if(strcmp(buffer, "/history\n") == 0)
                       {
                           get_history(sockfd, msg, buffer);
                       } else 
                       {
                           write(sockfd, msg, strlen(msg));   
                       }   
                   }          
                }   
             }
    }

    close(sockfd);

    return 0;
}

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

//#define PORT "3490"

#define BUFFERSIZE 128

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

    fd_set client_fds;                          // главный список дескрипторов
    fd_set use_fds;                             // временный список дескрипторов для селект
    int fdmax;                                  // максимальный номер дескриптора

    FD_ZERO(&client_fds);
    FD_ZERO(&use_fds);
    

    int sockfd, numbytes;
    struct addrinfo hints, *p, *servinfo;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char buffer[BUFFERSIZE];

    if (argc != 4) {
        fprintf(stderr,"usage: nickname ip port\n");
        return 1; 
    }

    char doc_true[] = "+++++++++++++++";
    char doc_false[] = "-";
    char *dv = ": ";
    char *PORT = argv[3];
    
    char t[BUFFERSIZE];
    memset(t, 0, BUFFERSIZE);

    char *nickname = strcat(t, argv[1]);
    nickname = strcat(nickname, dv);
    int nick_len = (int)strlen(nickname);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;



    if((rv = getaddrinfo(argv[2], PORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    for( p = servinfo; p != NULL; p = p->ai_next ){
        if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
            perror("client: socket\n");
            continue;
        }                                                   

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("client: connect\n");
            continue;
        }

        break;
    }

    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);

    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

///*
    FD_ZERO(&client_fds);
    FD_SET(sockfd, &client_fds);
    FD_SET(0, &client_fds);//stdin

    fdmax = sockfd + 1;

    int n;


    while (1) {
       use_fds = client_fds;
       select(fdmax, &use_fds, NULL,NULL,NULL);
       
       for(int fd = 0; fd < fdmax; fd++){
          if(FD_ISSET(fd, &use_fds)){
             if(fd == sockfd){   /*Accept data from open socket */
                //printf("client - read\n");
                //read data from open socket
                memset(buffer, 0, BUFFERSIZE);
                int result = read(sockfd, buffer, BUFFERSIZE);
                buffer[result] = '\0';  /* Terminate string with null */
                printf("%s", buffer);
             } else if(fd == 0){ /*process keyboard activiy*/
                //printf("%s", nickname);

                //memset(nickname + nick_len, 0, sizeof BUFFERSIZE - nick_len);
                //memset(buffer, 0, sizeof buffer);
                memset(buffer, 0, sizeof buffer);
                memset(nickname + nick_len, 0, (sizeof nickname) - nick_len);
                fgets(buffer, BUFFERSIZE, stdin);
                //char *all_msg = strcat(nickname, buffer);
                //printf("%s\n",kb_buffer);

                char *msg = strcat(nickname, buffer);

                if(strcmp(buffer, "\n"))
                {
                    if (strcmp(buffer, "quit\n") == 0) {
                        send(sockfd, msg, strlen(msg), 0);
                        printf("You're disconnect.\n");
                        close(sockfd); //close the current client
                        exit(0); //end program
                    } else if(strcmp(buffer, "/history\n") == 0)
                    {
                        write(sockfd, msg, strlen(msg));
                        read(sockfd, buffer, BUFFERSIZE);
                        int num = atoi(buffer);
                        printf("\nvvvv   history    vvvv\n");
                        int k = 0;

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

                            /*
                            if(t == 1)
                            {
                                printf("Sorry. Try again...\n");
                            }else
                            {
                                printf("%s", buffer);
                            }
                            //memset(buffer, 0, sizeof buffer);
                            */
                        }
                        printf("\n^^^^   history    ^^^^\n\n");
                    } else {
                        //printf("%s\n", msg);
                        send(sockfd, msg, strlen(msg), 0);   
                    }   
                    
                    memset(buffer, 0, sizeof buffer);
                    memset(nickname + nick_len, 0, (sizeof nickname) - nick_len);
                    //memset(buffer + nick_len, 0, BUFFERSIZE - nick_len);  
                }                                            
             }          
          }
       }      
     }
//*/

    close(sockfd);

    return 0;
}

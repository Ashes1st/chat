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

#define MAXDATASIZE 100
#define BUFFER_SIZE 256

void* session(void*);

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

    fd_set readfds, use_fds, client_fds;
    fd_set master;                              // главный список дескрипторов
    fd_set read_fds;                            // временный список дескрипторов для селект
    int fdmax;                                  // максимальный номер дескриптора

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    int sockfd, numbytes;
    struct addrinfo hints, *p, *servinfo;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char buffer[BUFFER_SIZE];

    if (argc != 4) {
        fprintf(stderr,"usage: nickname ip port\n");
        return 1; 
    }

    char *nickname = argv[1];
    int nick_len = (int)strlen(argv[1]);
    char *PORT = argv[3];

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
       select(FD_SETSIZE, &use_fds, NULL,NULL,NULL);
       
       for(int fd = 0; fd < FD_SETSIZE; fd++){
          if(FD_ISSET(fd, &use_fds)){
             if(fd == sockfd){   /*Accept data from open socket */
                //printf("client - read\n");
                //read data from open socket
                int result = read(sockfd, buffer, BUFFER_SIZE);
                buffer[result] = '\0';  /* Terminate string with null */
                printf("%s", buffer);
                memset(buffer, 0, sizeof buffer);
             } else if(fd == 0){ /*process keyboard activiy*/
                // printf("client - send\n");
                fgets(buffer, BUFFER_SIZE, stdin);
                //printf("%s\n",kb_buffer);
                if (strcmp(buffer, "quit\n") == 0) {
                    printf("You're disconnect.\n");
                    write(sockfd, buffer, strlen(buffer));
                    close(sockfd); //close the current client
                    exit(0); //end program
                } else {
                    //printf("%s", buffer);
                    write(sockfd, buffer, strlen(buffer));
                    memset(buffer, 0, sizeof buffer);
                }                                                 
             }          
          }
       }      
     }
//*/



/*
    while(1){
        printf("+");
        bzero(buffer, BUFFER_SIZE); //clear the buffer
        fgets(buffer, BUFFER_SIZE - 1, stdin);  //get the typing from the terminal
        printf("-\n");
        if(buffer[0] == '\n'){         //check if the entry is empty and if it is, goes into read mode
            printf("1\n");
            bzero(buffer, BUFFER_SIZE);
            read(sockfd, buffer, BUFFER_SIZE - 1);
            printf("%s\n",(char*) & buffer);
        } else{ //sends the message then reads then loops
            printf("2");
            bzero(buffer, BUFFER_SIZE);
            n = write(sockfd, buffer, strlen(buffer));
             if(n == 2  && buffer[0] == 'q'){   //checks if q was sent and quits if it was
                         exit(0);
                     }
            bzero(buffer, BUFFER_SIZE);
            n = read(sockfd, buffer, BUFFER_SIZE - 1);
            if(n < 0) {
                printf("Error: No message received");
            }
            printf("%s\n",(char*) & buffer);    
        }

    }
*/

/*    
    if((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1){
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: recived '%s'\n", buf);
*/
    close(sockfd);

    return 0;
}

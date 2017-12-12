//
//  server
//
//  Created by Ashe_s_ on 03.12.2017.
//  Copyright © 2017 ashes. All rights reserved.
//

/* server.c */


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
#include <sys/time.h>



//#define PORT "3490"     // Порт для подключения пользователей

#define BACKLOG 10      // Размер оереди для ожидающих подключения

#define BUFFERSIZE 128

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    
    if(i < 0){
        *p++ = '-';
        i *= -1;
    }
    
    int shifter = i;
    
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    
    *p = '\0';
    
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    
    return b;
}




int main(int argc, char *argv[])
{

    if (argc != 3) 
    {
        fprintf(stderr,"usage: port long_history\n");
        return 1; 
    }


/*--------------- Создаём все необходимые переменные ------------------*/
    char *PORT = argv[1];
    
    int LEN_HIS = atoi(argv[2]);                // Длина истории
    char *HIS = "/history\n";
    char history[LEN_HIS][BUFFERSIZE];
    int tec_pos_his = 0;
    int krug = 0;

    fd_set master;                              // главный список дескрипторов
    fd_set use_fds;                            // временный список дескрипторов для селект
    int fdmax;                                  // максимальный номер дескриптора

    FD_ZERO(&master);
    FD_ZERO(&use_fds);

    int listener;                               // дескриптор слушаемого сокета
    int new_fd;                                 // новопринятый дескриптор сокета
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage client_addr;
    socklen_t addrlen;
    
    int yes = 1;
    char ipstr[INET6_ADDRSTRLEN];

    char buffer[BUFFERSIZE];
    int nbytes;

    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;                  // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;            // type of connect
    hints.ai_flags = AI_CANONNAME;              // set your ip
    hints.ai_canonname = "127.0.0.1";           // set this ip
/*---------------------------------------------------------------------*/

/*------------- Заполняем servinfo информацией о нашей машине ---------*/
    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0 ){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
/*-----------------------------------------------------------------------*/


/*--------------------- создаём и биндим сокет для нашей машины ---------*/
    for( p = servinfo; p != NULL; p = p->ai_next ){
        if( (listener = socket(p->ai_family, p->ai_socktype, 0)) == -1 ){
            perror("server: socket\n");
            continue;
        }                                                   // Я открыл сокет
        

        /*
        if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }                                                   // Переподключаемся к процессам, которые после перезапуска ещё висят 
                                                            // и не дают создать новое 
                                                            // которые ещё висят в процессах
        */


        if(bind(listener, p->ai_addr, p->ai_addrlen) == -1){
            close(listener);
            perror("server: bind\n");
            continue;
        }

        break;
    }
/*-------------------------------------------------------------------*/

/*---------------------------- print ip:port ------------------------*/
    void *addr; 
    char *ipver;
                // получить,     
                // в IPv4 и IPv6 поля разные: 
        if (p->ai_family == AF_INET) {      // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4"; 
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr; 
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("%s: %s:%s\n", ipver, ipstr, PORT);
/*-------------------------------------------------------------------*/

    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo);

/*----------------- Слушаем --------------*/
    if(listen(listener, BACKLOG) == -1){
        perror("listen\n");
        exit(3);
    }




/*
    struct sigaction sa;

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
*/

    ///printf("socket %d\n", listener);
    FD_SET(listener, &master);
    fdmax = listener;

    printf("Server: waiting for connections...\n");

    while(1){

        use_fds = master;

        if(select(fdmax + 1, &use_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(4);
        }

        for(int i = 0; i <= fdmax; i++){

            if(FD_ISSET(i, &use_fds)){
                if(i == listener){
                    //обработка нового подключения
                    addrlen = sizeof client_addr;
                    new_fd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);

                    if(new_fd == -1)
                        perror("accept");
                    else{
                        FD_SET(new_fd, &master);

                        if(new_fd > fdmax){
                            fdmax = new_fd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d, fdmax = %d\n",
                            inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr), ipstr, INET6_ADDRSTRLEN), new_fd, fdmax);
                    }
                } else{
                    // обработка данных от клиента
                    memset(buffer, 0, sizeof buffer);
                    if((nbytes = recv(i, buffer, sizeof buffer, 0)) <= 0){
                        // error or disconnect
                        if(nbytes == 0){
                            printf("selectserver: socket %d hung up\n", i);
                        } else{
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &master);
                    } else
                        //printf("%d - %s", strncmp(buffer + nbytes - strlen(HIS), HIS, strlen(HIS)), buffer);
                        if(strcmp(buffer + nbytes - strlen(HIS), HIS) == 0) ////    HISTORY!!!!
                        {
                            if(krug == 0)
                            {
                                const int lh = strlen(argv[2]);
                                char ch[lh];
                                itoa(tec_pos_his, ch);
                                write(i, ch, strlen(ch));
                                int k = 0;

                                while(k < tec_pos_his)
                                {
                                    write(i, history[k], BUFFERSIZE);
                                    
                                    int w = 1;
                                    while(w == 1)
                                    {
                                        memset(buffer, 0, BUFFERSIZE);
                                        read(i, buffer, BUFFERSIZE);
                                        //printf("----%s-----\n", buffer);
                                        if(strlen(buffer) < 5)
                                        {
                                            write(i, history[k], BUFFERSIZE);
                                        }else
                                        {
                                            k++;
                                            w = 0;
                                        }
                                    }
                                    //printf("%d - %s", e, history[k]);
                                }
                            }else
                            {
                                write(i, argv[2], strlen(argv[2]));

                                int k = tec_pos_his;
                                while(k < LEN_HIS)
                                {
                                    write(i, history[k], BUFFERSIZE);
                                    
                                    int w = 1;
                                    while(w == 1)
                                    {
                                        memset(buffer, 0, BUFFERSIZE);
                                        read(i, buffer, BUFFERSIZE);
                                        if(strlen(buffer) < 5)
                                        {
                                            write(i, history[k], BUFFERSIZE);
                                        }else
                                        {
                                            k++;
                                            w = 0;
                                        }
                                    }
                                    //printf("%d - %s", e, history[k]);
                                }
                                
                                k = 0;

                                while(k < tec_pos_his)
                                {
                                    write(i, history[k], BUFFERSIZE);
                                    
                                    int w = 1;
                                    while(w == 1)
                                    {
                                        memset(buffer, 0, BUFFERSIZE);
                                        read(i, buffer, BUFFERSIZE);
                                        if(strlen(buffer) < 5)
                                        {
                                            write(i, history[k], BUFFERSIZE);
                                        }else
                                        {
                                            k++;
                                            w = 0;
                                        }
                                    }
                                    //printf("%d - %s", e, history[k]);
                                }
                            }
                        }
                        else
                        {

                            memmove(history[tec_pos_his], buffer, BUFFERSIZE);
                            tec_pos_his++;
                            if(tec_pos_his == LEN_HIS)
                            {
                                tec_pos_his = 0;
                                krug = 1;
                            }
                            


                                 // we got some data from client
                            printf("user %d say: %s", i, buffer);
                            for(int j = 0; (j <= fdmax); j++)
                            {
                                    //send all
                                if(FD_ISSET(j, &master))
                                {
                                    if((j != listener) && (j != i))
                                    {
                                        if(write(j, buffer, nbytes) == -1)
                                        {
                                            perror("send");
                                        }                                        
                                    }
                                }
                            } 
                        }  
                }
            }
        }

    }

    return 0;
}

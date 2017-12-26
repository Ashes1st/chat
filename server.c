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
#include <pthread.h>

#define BACKLOG 10      // Размер оереди для ожидающих подключения

#define BUFFERSIZE 128


typedef struct list
{
    int socket;
    
    struct list *next;
} list;

typedef struct Data
{
	int *run;
	int listen;
	int *fdmax;

	struct list **head;
	fd_set *master; 

} Data;


void list_push(list **head, int sock)
{
    if( !(*head) )
    {
        list *tmp = (list*) malloc(sizeof(list));
        tmp->next = NULL;
        tmp->socket = sock;
        (*head) = tmp;
    }else
    {
        list *tmp = (list*) malloc(sizeof(list));
        tmp->next = (*head);
        tmp->socket = sock;
        (*head) = tmp;
    }
}

void list_pop(list **head, int sock)
{
    list *tmp;
    list *pred = NULL;
    tmp = (*head);
    
    while(tmp->socket != sock)
    {
        pred = tmp;
        tmp = tmp->next;
    }
    
    if(!pred)
    {
        (*head) = (*head)->next;
    }
    else
    {
        pred->next = tmp->next;
    }
    
    free(tmp);
}

void sigchld_handler(int s)
{
	
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* itoa(int i, char b[])
{
	char const digit[] = "0123456789";
	char* p = b;
	
	if(i < 0)
	{
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

void print_ip(struct addrinfo *p, char PORT[])
{
	char ipstr[INET6_ADDRSTRLEN];
	void *addr; 
	char *ipver;
				// получить,     
				// в IPv4 и IPv6 поля разные: 
		if (p->ai_family == AF_INET) 
		{      // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4"; 
		} else 
		{ // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr; 
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("%s: %s:%s\n", ipver, ipstr, PORT);
}

int create_socket(int listener, char PORT[])
{
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int yes = 1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;                  // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;            // type of connect
	hints.ai_flags = AI_CANONNAME;              // set own ip
	hints.ai_canonname = "127.0.0.1";           // set this ip

	if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0 )
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	for( p = servinfo; p != NULL; p = p->ai_next )
	{
		if( (listener = socket(p->ai_family, p->ai_socktype, 0)) == -1 )
		{
			perror("server: socket\n");
			continue;
		}                                                   // Я открыл сокет
		
		if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}                                                   // Переподключаемся к процессам, которые после перезапуска ещё висят 
															// и не дают создать новое 
															// которые ещё висят в процессах
		if(bind(listener, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(listener);
			perror("server: bind\n");
			continue;
		}

		break;
	}

	if(p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}

	print_ip(p, PORT);

	freeaddrinfo(servinfo);

	return listener;
}

void kill_zombie()
{
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}
}

/*
void send_history(int socket, int krug, char *history, char *arg, char *buffer, int tec_pos_his, int len_his)
{
	if(krug == 0)
	{
		const int lh = strlen(arg);
		char ch[lh];
		itoa(tec_pos_his, ch);
		write(socket, ch, strlen(ch));
		int k = 0;

		while(k < tec_pos_his)
		{
			write(socket, history[k], BUFFERSIZE);
			
			int w = 1;
			while(w == 1)
			{
				memset(buffer, 0, BUFFERSIZE);
				read(socket, buffer, BUFFERSIZE);
				
				if(strlen(buffer) < 5)
				{
					write(socket, history[k], BUFFERSIZE);
				}else
				{
					k++;
					w = 0;
				}
			}
		}
	}else
	{
		write(socket, argv[2], strlen(argv[2]));

		int t = tec_pos_his;
		while(t < len_his)
		{
			write(socket, history[k], BUFFERSIZE);
			
			int w = 1;
			while(w == 1)
			{
				memset(buffer, 0, BUFFERSIZE);
				read(socket, buffer, BUFFERSIZE);
				
				if(strlen(buffer) < 5)
				{
					write(socket, history[t], BUFFERSIZE);
				}else
				{
					t++;
					w = 0;
				}
			}
		}
		
		t = 0;

		while(t < tec_pos_his)
		{
			write(socket, history[t], BUFFERSIZE);
			
			int w = 1;
			while(w == 1)
			{

				memset(buffer, 0, BUFFERSIZE);
				read(socket, buffer, BUFFERSIZE);
				if(strlen(buffer) < 5)
				{
					write(socket, history[t], BUFFERSIZE);
				}else
				{
					t++;
					w = 0;
				}
			}
		}
	}
}
*/

void *accept_members(void *data);


int main(int argc, char *argv[])
{	

	if (argc != 3) 
	{
		fprintf(stderr,"usage: port long_history\n");
		return 1; 
	}

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
	//struct addrinfo hints, *servinfo, *p;
	
	
	int Run = 1;
	Data *data = (Data*)malloc(sizeof(Data));
	data->run = &Run;
	data->fdmax = &fdmax;
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	
	char ipstr[INET6_ADDRSTRLEN];

	char buffer[BUFFERSIZE];
	int nbytes;
	
	listener = create_socket(listener, PORT);

	if(listener < 0)
	{
		printf("create_socket: ERROR");
		return 0;
	}

	printf("--socket %lu--\n", sizeof(list));


	list *head = (list*)malloc(sizeof(list));
	list *tec = (list*)malloc(sizeof(list));
	list *tec_send = (list*)malloc(sizeof(list));

	head = NULL;
	tec = NULL;
	tec_send = NULL;

	if(listen(listener, BACKLOG) == -1)
	{
		perror("listen\n");
		exit(3);
	}

	kill_zombie();

	FD_SET(0, &master); //stdin
	fdmax = 0;


	list_push(&head, 0);

	printf("listener - %d\n", listener);


	data->listen = listener;
	data->head = &head;
	data->master = &master;
	pthread_create(&thread, &attr, accept_members, data);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;

	printf("Server: waiting for connections...\n");

	while(Run)
	{
		use_fds = master;
		tec = head;

		if(select(fdmax + 1, &use_fds, NULL, NULL, &tv) == -1)
		{
			perror("select");
			exit(4);
		}

		int k = 1;

		while(k)
		{
			
			if(FD_ISSET(tec->socket, &use_fds))
			{
				memset(buffer, 0, sizeof buffer);
				if(tec->socket == 0)
				{
					fgets(buffer, BUFFERSIZE, stdin);

					if(strcmp(buffer, "/Down\n") == 0)
					{	
						Run = 0;
					}
				}
				else
				{
					if((nbytes = recv(tec->socket, buffer, sizeof buffer, 0)) <= 0)
					{
						// error or disconnect
						if(nbytes == 0)
						{
							printf("selectserver: socket %d hung up\n", tec->socket);
						} else
						{
							perror("recv");
						}

						close(tec->socket);
						FD_CLR(tec->socket, &master);

						list_pop(&head, tec->socket); 
					}
					else if(strcmp(buffer + nbytes - strlen(HIS), HIS) == 0) //History
					{
						//send_history(tec->socket, krug, history, argv[2], tec_pos_his, LEN_HIS);
						if(krug == 0)
                        {
                            const int lh = strlen(argv[2]);
                            char ch[lh];
                            itoa(tec_pos_his, ch);
                            write(tec->socket, ch, strlen(ch));
                            int i = 0;

                            while(i < tec_pos_his)
                            {
                                write(tec->socket, history[i], BUFFERSIZE);

                                int w = 1;
                                while(w == 1)
                                {
                                    memset(buffer, 0, BUFFERSIZE);
                                    read(tec->socket, buffer, BUFFERSIZE);
                                    
                                    if(strlen(buffer) < 5)
                                    {
                                        write(tec->socket, history[i], BUFFERSIZE);
                                        printf("2: %s\n", history[i]);
                                    }else
                                    {
                                        i++;
                                        w = 0;
                                    }
                                }
                            }
                        }else
                        {
                            write(tec->socket, argv[2], strlen(argv[2]));

                            int i = tec_pos_his;
                            while(i < LEN_HIS)
                            {
                                write(tec->socket, history[i], BUFFERSIZE);
                                
                                int w = 1;
                                while(w == 1)
                                {
                                    memset(buffer, 0, BUFFERSIZE);
                                    read(tec->socket, buffer, BUFFERSIZE);
                                    
                                    if(strlen(buffer) < 5)
                                    {
                                        write(tec->socket, history[i], BUFFERSIZE);
                                    }else
                                    {
                                        i++;
                                        w = 0;
                                    }
                                }
                            }
                            
                            i = 0;

                            while(i < tec_pos_his)
                            {
                                write(tec->socket, history[i], BUFFERSIZE);
                                
                                int w = 1;
                                while(w == 1)
                                {

                                    memset(buffer, 0, BUFFERSIZE);
                                    read(tec->socket, buffer, BUFFERSIZE);
                                    if(strlen(buffer) < 5)
                                    {
                                        write(tec->socket, history[i], BUFFERSIZE);
                                    }else
                                    {
                                        i++;
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

						tec_send = head;

						printf("user %d say: %s", tec->socket, buffer);

						int o = 1;
							
						while(o) //send all
						{
							if(FD_ISSET(tec_send->socket, &master))
							{
								if((tec_send->socket != listener) && (tec_send->socket != tec->socket))
								{
									if(write(tec_send->socket, buffer, nbytes) == -1)
									{
										perror("send");
									}                                        
								}
							}

							if(tec_send->next != NULL)
								tec_send = tec_send->next;
							else
								o = 0;
						}
					}
				}
			}


			if(tec->next != NULL)
				tec = tec->next;
			else
				k = 0;
		}
	}

	printf("Wait thread...\n");

	int result_join = pthread_join(thread, NULL);

	if(result_join == 0)
		printf("\nSuccess\n");
	else
		printf("\nFault\n");


	// already free
	//free(head);
	//free(tec);
	
	free(tec_send);
	free(data);

	return 0;
}

void *accept_members(void *d)
{
	Data *data = (Data*)d;
	fd_set use_fds;
	fd_set master;
	FD_ZERO(&use_fds);
	int new_fd;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;

	struct sockaddr_storage client_addr;
	socklen_t addrlen;

	FD_SET(data->listen, &master);

	while(*(data->run))
	{
		use_fds = master;

		if(select(data->listen + 1, &use_fds, NULL, NULL, &tv) > 0)
		{
			if(FD_ISSET(data->listen, &use_fds))
			{		

				//обработка нового подключения
				addrlen = sizeof client_addr;
				new_fd = accept(data->listen, (struct sockaddr*)&client_addr, &addrlen);

				if(new_fd == -1)
				{
					perror("accept");
				}else
				{
					FD_SET(new_fd, &(*(data->master)));

					list_push(data->head, new_fd);

					if(new_fd > *(data->fdmax))
					{
						*(data->fdmax) = new_fd;
					}
					
					printf("selectserver: new connection on "
						"socket %d, fdmax = %d\n", new_fd, *(data->fdmax));
				}
		
			}
		}
	}

	pthread_exit(0);
}











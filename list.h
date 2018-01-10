#ifndef __LIST_H
#define __LIST_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

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

void list_push(list **head, int sock);

void list_pop(list **head, int sock);

#endif

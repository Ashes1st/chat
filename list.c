#ifndef __STDLIB_H
#define __STDLIB_H

#include <stdlib.h> 

#endif

#include "variables.h"

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


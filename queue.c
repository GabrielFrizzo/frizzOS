//
// Created by frizzo on 02/03/2020.
//

#include "queue.h"
#include <stdio.h>

void _error(char* message) {
    printf("Erro na utilizacao da fila: %s\n", message);
}

void queue_append(queue_t **queue, queue_t *elem) {
    // - a fila deve existir
    if (queue == NULL) {_error("A fila deve existir"); return;}
    // - o elemento deve existir
    if(elem == NULL) {_error("O elemento deve existir"); return;}
    // - o elemento nao deve estar em outra fila
    if(elem->next != NULL) {_error("O elemento nao deve estar em outra fila"); return;}

    if (*queue == NULL) {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
    }

    queue_t* first = *queue, *last = first->prev;

    last->next = elem;
    elem->prev = last;
    elem->next = first;
    first->prev = elem;
}

queue_t *queue_remove(queue_t **queue, queue_t *elem) {
    // - a fila deve existir
    if(queue == NULL) {_error("a fila deve existir");return NULL;}
    // - o elemento deve existir
    if(elem == NULL) {_error("o elemento deve existir");return NULL;}
    // - o elemento nao deve estar em outra fila
    //    if(elem->next != NULL) return NULL;

    queue_t* first = *queue, *prev = first;
    if (first == NULL) {
        _error("tentou remover de fila vazia");return NULL;
    }

    if (elem == first) {
        if(elem == first->next) {
            *queue = NULL;
            elem->next = elem->prev = NULL;
            return elem;
        }
        *queue = first->next;
    }

    while(prev->next != elem) {
        prev = prev->next;

        // - o elemento deve pertencer a fila indicada
        if (prev == first) {
            _error("o elemento deve pertencer a fila indicada");return NULL;
        }
    }

    queue_t *next = elem->next;

    elem->prev = elem->next = NULL;
    prev->next = next;
    next->prev = prev;

    return elem;
}

int queue_size(queue_t *queue) {
    if (queue == NULL) return 0;
    queue_t* iterator = queue;
    int count = 0;
    do{
        iterator = iterator->next;
        count++;
    }while(iterator != queue);

    return count;
}

void queue_print(char *name, queue_t *queue, void (*print_elem)(void *)) {
    queue_t *curr = queue;
    // tem que printar isso?
    printf("[");
    if (curr != NULL){
        do {
            print_elem(curr);
            curr = curr->next;
            if (curr != queue) printf(" ");
        } while (curr != queue);
    }
    printf("]\n");
}


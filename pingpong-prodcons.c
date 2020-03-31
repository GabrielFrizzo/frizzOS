//
// Created by frizzo on 30/03/2020.
//

#include "pingpong.h"

#define MAXBUFSIZE 5


typedef struct buffitem {
    struct buffitem *prev, *next;
    int value;
} buffitem;


buffitem *buffer;

semaphore_t s_buffer, s_item, s_vaga;
task_t p1, p2, p3, c1, c2;


void produtor(void* arg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"   // remove warning de loop infinito
    while(1) {
        task_sleep(1);
//        printf("%s aguardando vaga\n", arg);
        sem_down(&s_vaga);
//        printf("%s conseguiu vaga\n", arg);

//        printf("%s aguardando buffer\n", arg);
        sem_down(&s_buffer);
//        printf("%s conseguiu buffer\n", arg);
        buffitem *item = malloc(sizeof(buffitem));
        item->value = rand()%100;
        queue_append((queue_t**) &buffer, (queue_t*) item);

        sem_up(&s_buffer);
//        printf("%s liberou buffer\n", arg);

        sem_up(&s_item);
//        printf("%s liberou item\n", arg);
        printf("%s produziu %d\n", arg, item->value);
    }
#pragma clang diagnostic pop
}

void consumidor(void* arg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while(1) {
//        printf("%s aguardando item\n", arg);
        sem_down(&s_item);
//        printf("%s conseguiu item\n", arg);

//        printf("%s aguardando buffer\n", arg);
        sem_down(&s_buffer);
//        printf("%s conseguiu buffer\n", arg);
        int value = buffer->value;
        queue_remove((queue_t**) &buffer, (queue_t*) buffer);
        sem_up(&s_buffer);
//        printf("%s liberou buffer\n", arg);

        sem_up(&s_vaga);
//        printf("%s liberou vaga\n", arg);
        printf("%s consumiu %d\n", arg, value);
        task_sleep(1);
    }
#pragma clang diagnostic pop
}

int main(int argc, char *argv[]) {
    pingpong_init();

    sem_create(&s_buffer, 1);
    sem_create(&s_item, 0);
    sem_create(&s_vaga, 5);

    task_create(&p1, produtor, "p1");
    task_create(&p2, produtor, "p2");
    task_create(&p3, produtor, "p3");
    task_create(&c1, consumidor, "\t\tc1");
    task_create(&c2, consumidor, "\t\tc2");

    task_join(&c1);

    sem_destroy(&s_buffer);
    sem_destroy(&s_item);
    sem_destroy(&s_vaga);

    task_exit(0);
    return 0;
}
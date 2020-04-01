//
// Created by frizzo on 31/03/2020.
//

#include "pingpong.h"

int mqueue_create(mqueue_t *queue, int max, int size) {
    semaphore_t *sem_msg, *sem_vaga, *sem_buffer;
    sem_msg = malloc(sizeof(semaphore_t));
    sem_vaga = malloc(sizeof(semaphore_t));
    sem_buffer = malloc(sizeof(semaphore_t));
    sem_create(sem_msg, 0);
    sem_create(sem_vaga, max);
    sem_create(sem_buffer, 1);

    queue->msg_size = size;
    queue->sem_msg = sem_msg;
    queue->sem_vaga = sem_vaga;
    queue->sem_buffer = sem_buffer;

    return 0;
}

int mqueue_send(mqueue_t *queue, void *msg) {
//    printf("%d aguardando vaga\n", currTask->tid);
    sem_down(queue->sem_vaga);
//    printf("%d conseguiu vaga\n", currTask->tid);

//    printf("%d aguardando buffer\n", currTask->tid);
    sem_down(queue->sem_buffer);
//    printf("%d conseguiu buffer\n", currTask->tid);
    if (queue->destroyed) return -1;

    // envia a mensagem
    message_t *pMessage = malloc(sizeof(message_t));
    pMessage->content = malloc(queue->msg_size);
    pMessage->size = queue->msg_size;
    memcpy(pMessage->content, msg, pMessage->size);
    queue_append((queue_t **) &queue->msg_q, (queue_t *) pMessage);

//    printf("%d liberou buffer\n", currTask->tid);
    sem_up(queue->sem_buffer);

//    printf("%d liberou mensagem\n", currTask->tid);
    sem_up(queue->sem_msg);
    return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg) {
    sem_down(queue->sem_msg);

    sem_down(queue->sem_buffer);

    if (queue->destroyed) return -1;
    // envia a mensagem
    message_t *pMessage = queue->msg_q;
    queue_remove((queue_t**) &queue->msg_q, (queue_t*) pMessage);
    memcpy(msg, pMessage->content, pMessage->size);

    sem_up(queue->sem_buffer);

    sem_up(queue->sem_vaga);
    return 0;
}

int mqueue_destroy(mqueue_t *queue) {
    sem_destroy(queue->sem_msg);
    sem_destroy(queue->sem_vaga);
    sem_destroy(queue->sem_buffer);
    queue->destroyed = 1;

    return 0;
}

int mqueue_msgs(mqueue_t *queue) {
    return queue->sem_msg->value;
}
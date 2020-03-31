//
// Created by frizzo on 30/03/2020.
//

#include "pingpong.h"


// preemption lock

void _enter_crit_sec() {
    lock = 1;
}

void _leave_crit_sec() {
    lock = 0;
}

// semaphore methods

int sem_create(semaphore_t *s, int value) {
    s->value = value;
    s->destroyed = 0;
    return 0;
}

int sem_down (semaphore_t *s) {
    if (s == NULL || s->destroyed) return -1;

    _enter_crit_sec();
    if (--s->value < 0) {
        task_suspend(NULL, &s->task_q);
        _leave_crit_sec();
        task_yield();
    }else{
        _leave_crit_sec();
    }


    return -s->destroyed; // -1 if sem was destroyed
}

int sem_up (semaphore_t *s) {
    if (s == NULL || s->destroyed) return -1;

    _enter_crit_sec();
    if (++s->value <= 0) {
        task_resume(s->task_q);
    }
    _leave_crit_sec();

    return 0;
}


int sem_destroy (semaphore_t *s) {
    if (s == NULL) return -1;
    s->destroyed = 1;
    if (s->task_q == NULL) return 0;

    task_t* toResume[1000]; // to avoid removing while iterating
    int cntToResume = 0;
    task_t *aux = s->task_q;
    do {
        toResume[cntToResume++] = aux;
        aux = aux->next;
    } while(aux != s->task_q);

    for (int i = 0; i < cntToResume; ++i) task_resume(toResume[i]); // resume w error code?
    return 0;
}

// barrier methods

int barrier_create(barrier_t *b, int N) {
    semaphore_t bsem;
    sem_create(&bsem, 1);
    b->sem = &bsem;
    b->count = N;
    return 0;
}

int barrier_join(barrier_t *b) {
    sem_down(b->sem);
#ifdef DEBUG
    printf("D: task %d entrou na barreira. faltam %d\n", currTask->tid, b->count-1);
#endif
    if(--b->count) {
        task_suspend(NULL, &b->task_q);
        task_yield();
    } else {
        barrier_destroy(b);
    }

    sem_up(b->sem);

    return 0;
}

int barrier_destroy(barrier_t *b) {
    task_t* toResume[1000]; // to avoid removing while iterating
    int cntToResume = 0;
    task_t *aux = b->task_q;
    do {
        toResume[cntToResume++] = aux;
        aux = aux->next;
    } while(aux != b->task_q);

    for (int i = 0; i < cntToResume; ++i) task_resume(toResume[i]); // resume w error code?

    return 0;
}

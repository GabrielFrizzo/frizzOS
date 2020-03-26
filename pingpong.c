//
// Created by frizzo on 25/03/2020.
//


#include "pingpong.h"

// variáveis globais
int lastTaskID ;
task_t mainTask, *currTask;

void _create_context(ucontext_t* context) {
    getcontext(context);
    char* stack = malloc(STACKSIZE) ;
    if (stack)
    {
        context->uc_stack.ss_sp = stack ;
        context->uc_stack.ss_size = STACKSIZE;
        context->uc_stack.ss_flags = 0;
        context->uc_link = 0;
    }
    else
    {
        perror ("Erro na criacao da pilha: ");
        exit (1);
    }
}


// implementacao das funcoes publicas
void pingpong_init() {
//    ucontext_t contextMain;
//    _create_context(&contextMain);

//    mainTask.context = contextMain;

    lastTaskID = 0;
    mainTask.tid = lastTaskID++;

    currTask = &mainTask;

    setvbuf(stdout, 0, _IONBF, 0);
}

int task_create(task_t *task, void (*start_func)(void*), void *arg) {
    ucontext_t newContext;
    _create_context(&newContext);

    makecontext(&newContext, start_func, 1, arg);

    task->context = newContext;
    return task->tid = lastTaskID++;
}

int task_switch(task_t *task) {
    #ifdef DEBUG
        printf("Task Switched %d -> %d\n", currTask->tid, task->tid);
    #endif //DEBUG print
    ucontext_t *currContext = &currTask->context;
    currTask = task;
    swapcontext(currContext, &task->context);
    return 0;
}

void task_exit(int exitCode) {
    task_switch(&mainTask);
}

int task_id() {
    return currTask->tid;
}


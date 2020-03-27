//
// Created by frizzo on 25/03/2020.
//


#include "pingpong.h"

#define DEFAULT_PRIO 0

// variáveis globais
int lastTaskID ;
task_t mainTask, *currTask, dispatcher;
task_t *readyTasks, *suspendedTasks;

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

// scheduler algorithms
task_t* _fcfs(task_t *taskQueue) {
    return taskQueue;
}

task_t* _aging_prio(task_t *taskQueue) {
    int lowPrio = 21;
    task_t *first, *aux, *bestTask;
    first = aux = bestTask = taskQueue;
    do {
        if (aux->dPrio < lowPrio) {
            lowPrio = aux->dPrio;
            bestTask = aux;
        }
        aux->dPrio--; // max de 20?
        aux = aux->next;
    } while(aux != first);

    bestTask->dPrio = bestTask->ePrio;
    return bestTask;
}

task_t* _scheduler() {
    return _aging_prio(readyTasks);
}

void _append_ready_task(task_t* task) {
    queue_append((queue_t**) &readyTasks, (queue_t*) task);
#ifdef DEBUG
    queue_print("asdas", (queue_t*) readyTasks, _print_elem);
#endif
}

void _dispatcher_exec(void *arg) {
    while (queue_size((queue_t*)readyTasks) > 0) {
        task_t *nextTask = _scheduler();
        if (nextTask != NULL) {
            task_switch(nextTask);
            switch (nextTask->status) {
                case READY:
                    queue_remove((queue_t **) &readyTasks, (queue_t *) nextTask);
                    _append_ready_task(nextTask);
                    break;
                case TERMINATED:
                    queue_remove((queue_t **) &readyTasks, (queue_t *) nextTask);
                    free(nextTask->stack);
                    break;
                default:
                    break;
            }
        }
    }
    task_exit(0);
}

void _print_elem (void *ptr)
{
    task_t *elem = ptr ;

    if (!elem)
        return ;

    elem->prev ? printf ("%d", elem->prev->tid) : printf ("*") ;
    printf ("<%d>", elem->tid) ;
    elem->next ? printf ("%d", elem->next->tid) : printf ("*") ;
}

// implementacao das funcoes publicas
void pingpong_init() {
    setvbuf(stdout, 0, _IONBF, 0);

    lastTaskID = 0;
    mainTask.tid = lastTaskID++;

    currTask = &mainTask;

    task_create(&dispatcher, &_dispatcher_exec, NULL);
    dispatcher.parent = &mainTask;
    queue_remove((queue_t **) &readyTasks, (queue_t *) &dispatcher);
}

int task_create(task_t *task, void (*start_func)(void*), void *arg) {
    ucontext_t newContext;
    _create_context(&newContext);

    makecontext(&newContext, start_func, 1, arg);

    _append_ready_task(task);

    task->tid = lastTaskID++;
    task->context = newContext;
    task->stack = newContext.uc_stack.ss_sp;
    task->parent = &dispatcher;
    task->status = READY;
    task->ePrio = task->dPrio = DEFAULT_PRIO;
    return task->tid;
}

int task_switch(task_t *task) {
    #ifdef DEBUG
        printf("Task Switched %d -> %d\n", currTask->tid, task->tid);
    #endif //DEBUG print
    task_t *prevTask = currTask;
    currTask = task;
    if (swapcontext(&prevTask->context, &task->context) != 0) {
        perror("Erro na troca de contextos");
    }
    return 0;
}

void task_exit(int exitCode) {
    currTask->status = TERMINATED;
    task_switch(currTask->parent);
}

int task_id() {
    return currTask->tid;
}

void task_yield() {
    task_switch(&dispatcher);
}

void task_suspend(task_t *task, task_t **queue) {
    if (queue == NULL) { return; }
    if (task == NULL) { task=currTask; }
    queue_remove((queue_t **) &readyTasks, (queue_t *) task);
    queue_append((queue_t **) &suspendedTasks, (queue_t *) task);
    task->status = SUSPENDED;
}

void task_resume(task_t *task) {
    queue_remove((queue_t **) &suspendedTasks, (queue_t *) task);
    queue_append((queue_t **) &readyTasks, (queue_t *) task);
    task->status = READY;
}

void task_setprio(task_t *task, int prio) {
    if (prio > 20) { prio = 20; }
    if (prio < -20) { prio = -20; }
    task_t *toset = task == NULL ? currTask : task;
    toset->dPrio = toset->ePrio = prio;
}

int task_getprio(task_t *task) {
    return task == NULL ? currTask->ePrio : task->ePrio;
}


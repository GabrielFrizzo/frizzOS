//
// Created by frizzo on 25/03/2020.
//


#include "pingpong.h"

#define DEFAULT_PRIO 0
#define DEFAULT_TICKS 20
#define WAKE_PERIOD 250 // time to check sleeping tasks (in ms)



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
    if (taskQueue == NULL) return NULL;
    int lowPrio = 21;
    task_t *first, *aux, *bestTask;
    first = aux = bestTask = taskQueue;
    do {
        if (aux->dPrio < lowPrio) {
            lowPrio = aux->dPrio;
            bestTask = aux;
        }
        if (aux->dPrio > -20) aux->dPrio--; // min de 20?
        aux = aux->next;
    } while(aux != first);

    bestTask->dPrio = bestTask->ePrio;
    return bestTask;
}

task_t* _scheduler() {
    return _aging_prio(readyTasks);
}

void _append_ready_task(task_t* task) {
    task->queue = &readyTasks;
    queue_append((queue_t**) &readyTasks, (queue_t*) task);
#ifdef DEBUG
    queue_print("asdas", (queue_t*) readyTasks, _print_elem);
#endif
}

void _handle_tick() {
    msec++;
#ifdef DEBUG
    printf("Handle tick\ttid: %d\tticks left: %d\n", currTask->tid, currTask->ticksLeft);
#endif
    if (!currTask->isUserTask) return;

    if (--currTask->ticksLeft <= 0 && !lock) {
        task_yield();
    }
}

void _start_timer() {
    action.sa_handler = _handle_tick;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
    msec = 0;
}

void _wake_tasks() {
    if (sleepingTasks == NULL) return; // empty suspended list

    task_t *iter = sleepingTasks;
    task_t* toWake[1000]; // to avoid removing while iterating
    int cntToWake = 0;
    do {
        if (systime() >= iter->wakingTime) toWake[cntToWake++] = iter;
        iter = iter->next;
    } while(iter != sleepingTasks);

    for(int i = 0; i < cntToWake; i++) task_resume(toWake[i]);
}

void _dispatcher_exec(void *arg) {
    while (readyTasks != NULL || sleepingTasks != NULL || suspendedTasks != NULL) {
        if (systime() % WAKE_PERIOD == 0) _wake_tasks();

        task_t *nextTask = _scheduler();
        if (nextTask != NULL) {
            // pre process task
            switch (nextTask->status) {
                case SUSPENDED:
                    task_suspend(nextTask, &suspendedTasks);
                    break;
                case TERMINATED:
                    queue_remove((queue_t **) &readyTasks, (queue_t *) nextTask);
                    free(nextTask->stack);
                    break;
                default:
                    break;
            }

            task_switch(nextTask);

            // post process task
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

    // main init
    mainTask.tid = lastTaskID++;
    mainTask.parent = &dispatcher;
    mainTask.isUserTask = 1;
    mainTask.status = READY;
    mainTask.ePrio = mainTask.dPrio = DEFAULT_PRIO; // perguntar se deve ser default
    mainTask.perf.gStartTime = mainTask.perf.lStartTime = systime();
    mainTask.perf.activations = mainTask.perf.totalPTime = 0;
    mainTask.queue = &readyTasks;
    _append_ready_task(&mainTask);
    currTask = &mainTask;


    // init dispatcher
    task_create(&dispatcher, &_dispatcher_exec, NULL);
    dispatcher.parent = &mainTask;
    dispatcher.isUserTask = 0;
    queue_remove((queue_t **) &readyTasks, (queue_t *) &dispatcher);

    _start_timer();

    task_yield();
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
    task->isUserTask = 1;
    task->perf.gStartTime = task->perf.lStartTime = systime();
    task->perf.activations = task->perf.totalPTime = 0;
    return task->tid;
}

int task_switch(task_t *task) {
    #ifdef DEBUG
        printf("Task Switched %d -> %d\n", currTask->tid, task->tid);
    #endif //DEBUG print
    task_t *prevTask = currTask;
    currTask = task;

    prevTask->perf.totalPTime += systime() - prevTask->perf.lStartTime;
    currTask->perf.activations++;
    currTask->perf.lStartTime = systime();
    currTask->ticksLeft = DEFAULT_TICKS;
    if (swapcontext(&prevTask->context, &task->context) != 0) {
        perror("Erro na troca de contextos");
    }
    return 0;
}

void _activate_joined_tasks(task_t *pTask) {
    if (suspendedTasks == NULL) return; // empty suspended list

    task_t *iter = suspendedTasks;
    task_t* toSuspend[1000]; // to avoid removing while iterating
    int cntToSuspend = 0;
    do {
        if (iter->joined == pTask) toSuspend[cntToSuspend++] = iter;
        iter = iter->next;
    } while(iter != suspendedTasks);

    for(int i = 0; i < cntToSuspend; i++) task_resume(toSuspend[i]);
}

void task_exit(int exitCode) {
    printf("Task %d exit: execution time %u ms, processor time %u ms, %d activations\n",
            currTask->tid,
            systime() - currTask->perf.gStartTime,
            currTask->perf.totalPTime,
            currTask->perf.activations
            );

    currTask->status = TERMINATED;
    currTask->exitCode = exitCode;

    _activate_joined_tasks(currTask);
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
    task->queue = queue;
    queue_remove((queue_t **) &readyTasks, (queue_t *) task);
    queue_append((queue_t **) queue, (queue_t *) task);
    task->status = SUSPENDED;
}

void task_resume(task_t *task) {
    queue_remove((queue_t **) task->queue, (queue_t *) task);
    queue_append((queue_t **) &readyTasks, (queue_t *) task);
    task->status = READY;
#ifdef DEBUG
    printf("D: resumed task %d\n", task->tid);
#endif
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

unsigned int systime() {
    return msec;
}

int task_join(task_t *task) {
    if (task == NULL || task->status == TERMINATED) return -1;
#ifdef DEBUG
    printf("D: task %d waiting for %d\n", currTask->tid, task->tid);
#endif
    currTask->joined = task;
    task_suspend(currTask, &suspendedTasks);
    task_yield();
#ifdef DEBUG
    printf("D: task %d no longer waiting for %d\n", currTask->tid, task->tid);
#endif
    return task->exitCode;
}

void task_sleep(int t) {
    queue_remove((queue_t **) &readyTasks, (queue_t *) currTask);
    queue_append((queue_t **) &sleepingTasks, (queue_t *) currTask);
    currTask->status = SLEEPING;
    currTask->wakingTime = systime() + t*1000;
    currTask->queue = &sleepingTasks;
    task_yield();
}


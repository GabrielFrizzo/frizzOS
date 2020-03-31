// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "queue.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */

enum status_t { READY, SUSPENDED, TERMINATED, SLEEPING };

// Estrutura que define informacoes de uso do processador de tarefas
typedef struct performance
{
    unsigned int activations;
    unsigned int gStartTime, lStartTime; // global and local start time
    unsigned int totalPTime; //Total processor time
} performance;

// Estrutura que define uma tarefa
typedef struct task_t
{
    struct task_t *prev, *next;
    int tid;
    ucontext_t context;
    void *stack;
    struct task_t *parent;
    enum status_t status;
    int ePrio;
    int dPrio;
    int ticksLeft;
    int isUserTask; //bool
    performance perf;
    struct task_t *joined;
    int exitCode;
    unsigned int wakingTime;
    struct task_t **queue;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
    task_t *task_q; //task queue
    int value;
    int destroyed;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
  // NAO É NECESSÁRIO?
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  semaphore_t *sem;
  task_t *task_q;
  int count;
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

// variáveis globais
int lastTaskID ;
task_t mainTask, *currTask, dispatcher;
task_t *readyTasks, *suspendedTasks, *sleepingTasks;
struct sigaction action;
struct itimerval timer;
unsigned int msec;
int lock;

#endif

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// interface do driver de disco rígido

#ifndef __DISKDRIVER__
#define __DISKDRIVER__

#include "harddisk.h"
#include "pingpong.h"

// estrutura que representa um pedido de acesso ao disco
typedef struct disk_request{
    struct disk_request *prev, *next;
    task_t *task;
    int op;         // DISK_CMD_{READ,WRITE}
    int block;
    void* buffer;
} disk_request;

// structura de dados que representa o disco para o SO
typedef struct
{
  semaphore_t sem_disk;     // semaphore for disk access
  struct sigaction action;  // SIGUR1 handler
  disk_request *req_q;      // queue of requests waiting to be occupy disk
  disk_request *currReq;    // request currently occupying disk
  int diskRDY;              // bool to indicate IO is ready
} disk_t ;

// inicializacao do driver de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int diskdriver_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer indicado
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer indicado para o disco
int disk_block_write (int block, void *buffer) ;

// variáveis globais
task_t diskManager;
disk_t disk;

#endif

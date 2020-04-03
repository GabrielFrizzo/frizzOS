//
// Created by frizzo on 02/04/2020.
//

#include "diskdriver.h"


void _sigusr1_handler() {
    sem_down(&disk.sem_disk);
//    printf("D: disk returned for task %d\n", disk.currReq->task->tid);
    disk.diskRDY = 1;
    task_resume(&diskManager);
    sem_up(&disk.sem_disk);
}

disk_request* _dsk_fcfs(disk_request *pRequest) {
    return pRequest;
}

disk_request* _sstf(disk_request *pRequest) {
    disk_request* iterator = pRequest, *best = iterator;
    int min = 1<<30;
    do {
        int dist = abs(iterator->block - disk.currBlock);
        if (dist < min) {
            min = dist;
            best = iterator;
        }
        iterator = iterator->next;
    } while(iterator!= pRequest);

    return best;
}

disk_request *_cscan(disk_request *pRequest) {
    disk_request* iterator = pRequest, *best = NULL;
    int foundNxt = 0;
    int minDist = 1<<30, minBlk = minDist;
    do {
        int dist = iterator->block - disk.currBlock;
        if (dist < minDist && dist >= 0) {
            foundNxt = 1;
            minDist = dist;
            best = iterator;
        } else if (!foundNxt && iterator->block < minBlk) { // keeps track of lowest position
            minBlk = iterator->block;                       // in case there's none left ahead
            best = iterator;
        }
        iterator = iterator->next;
    } while(iterator!= pRequest);

    return best;
}

disk_request *_disk_schedule() {
    switch (DSK_SCHD_ALG) {
        case FCFS:
            return _dsk_fcfs(disk.req_q);
        case SSTF:
            return _sstf(disk.req_q);
        case CSCAN:
            return _cscan(disk.req_q);
        default:
            return _dsk_fcfs(disk.req_q);

    }
    return NULL;
}

void _disk_manager_execute(void* arg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (mainTask.status != TERMINATED) {
        sem_down(&disk.sem_disk);

        if (disk.diskRDY) {
//            printf("D: resuming %d\n", disk.currReq->task->tid);
            disk.diskRDY = 0;
            task_resume(disk.currReq->task);
            queue_remove((queue_t**) &disk.req_q, (queue_t*) disk.currReq);
            free(disk.currReq);
            disk.currReq = 0;
        }

        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && disk.req_q != NULL) {
//            printf("D: curr req: %d\n", disk.req_q->task->tid);
            disk.currReq = _disk_schedule();
            disk.totalBlkRun += abs(disk.currReq->block - disk.currBlock);
            disk.currBlock = disk.currReq->block;
            disk_cmd(disk.currReq->op, disk.currReq->block, disk.currReq->buffer);
        }

        sem_up(&disk.sem_disk);
//        task_suspend(NULL, &suspendedTasks);
//        task_yield();
        task_join(&mainTask);
    }
#pragma clang diagnostic pop
    printf("total block run: %d blocks\n", disk.totalBlkRun);
    task_exit(0);
}

int diskdriver_init(int *numBlocks, int *blockSize) {
    // set up SIGUSR1 handling
    disk.action.sa_handler = _sigusr1_handler;
    sigemptyset (&disk.action.sa_mask) ;
    disk.action.sa_flags = 0 ;
    if (sigaction (SIGUSR1, &disk.action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // init disk access semaphore
    sem_create(&disk.sem_disk, 1);

    //init harddisk
    disk.currReq = 0;
    disk.diskRDY = 0;
    disk.currBlock = 0;
    disk.totalBlkRun = 0;
    disk_cmd(DISK_CMD_INIT, 0, 0);
    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

    // init disk manager task
    task_create(&diskManager, _disk_manager_execute, "");
    return 0;
}

int _disk_op(int block, void *buffer, int op) {
    sem_down(&disk.sem_disk);

    disk_request* req = malloc(sizeof(disk_request));
    req->block = block;
    req->buffer = buffer;
    req->task = currTask;
    req->op = op;
    queue_append((queue_t**) &disk.req_q, (queue_t*) req);

    if (diskManager.status == SUSPENDED)
        task_resume(&diskManager);

    task_suspend(currTask, &suspendedTasks);
    sem_up(&disk.sem_disk);

    task_yield();
    return 0;
}

int disk_block_read(int block, void *buffer) {
    return _disk_op(block, buffer, DISK_CMD_READ);
}

int disk_block_write(int block, void *buffer) {
    return _disk_op(block, buffer, DISK_CMD_WRITE);
}

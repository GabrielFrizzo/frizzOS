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

void _disk_manager_execute(void* arg) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1) {
        sem_down(&disk.sem_disk);

        if (disk.diskRDY) {
//            printf("D: resuming %d\n", disk.currReq->task->tid);
            disk.diskRDY = 0;
            task_resume(disk.currReq->task);
            queue_remove((queue_t**) &disk.req_q, (queue_t*) disk.currReq);
            free(disk.currReq);
            disk.currReq = 0;
        }

        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE &&
            queue_size((queue_t*) disk.req_q) > 0)
        {
//            printf("D: curr req: %d\n", disk.req_q->task->tid);
            disk.currReq = disk.req_q;
            disk_cmd(disk.currReq->op, disk.currReq->block, disk.currReq->buffer);
        }

        sem_up(&disk.sem_disk);
        task_suspend(NULL, &suspendedTasks);
        task_yield();
    }
#pragma clang diagnostic pop
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
    disk.diskRDY= 0;
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

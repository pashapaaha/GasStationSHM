#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE 2000
#define ROM_76_1_SEM 2001
#define ROM_76_2_SEM 2002
#define ROM_92_1_SEM 2003
#define ROM_92_2_SEM 2004
#define ROM_95_1_SEM 2005

#define PERMS 0666

typedef struct{
    int car_id;
    int fuel_type;
} Car;


void* start_terminal(void*);
void sys_err(const char*);

int main(int argc, char** argv){


    return 0;
}

void* start_terminal(void* arg){
    const int SEM_ID = * (int*) arg;
    int semid, shmid;
    Car* car;
    if ((semid = semget(SEM_ID, 1, PERMS | IPC_CREAT)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(Car), PERMS | IPC_CREAT)) < 0) {
        sys_err("server: can not get shared memory segment");
    }
    if ((car = (Car *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("server: shared memory attach error");
    }

    semctl(semid, 0, SETVAL, 0);
    while(1){
        if(semctl(semid, 0, GETVAL, 0))
            continue;

        semctl(semid, 0, SETVAL, 1);

        /*TODO: Реализовать соответствующую логику обработки машины
         * Использовать sleep()*/


        semctl(semid, 0, SETVAL, 0);
        if(true) break;
    }

    if(semctl(semid, 0, IPC_RMID, (struct semid_ds*)0) < 0){
        sys_err("server: semaphore remove error");
    }

    shmdt(car);
    if(shmctl(shmid, IPC_RMID, (struct shmid_ds*)0) < 0){
        sys_err("server: shared memory remove error");
    }
    exit(0);
}

void sys_err(const char *msg) {
    puts(msg);
    exit(1);
}
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
#include <list>

#define QUEUE 2000
#define ROM_76_1_SEM 2001
#define ROM_76_2_SEM 2002
#define ROM_92_1_SEM 2003
#define ROM_92_2_SEM 2004
#define ROM_95_1_SEM 2005

#define PERMS 0666

typedef struct {
    int car_id;
    int fuel_type;
} Car;


void *start_terminal(void *);

void *start_cars_flow(void *);

void sys_err(const char *);

unsigned int serviceTime;
unsigned int period;
unsigned int maxSize;

int main(int argc, char **argv) {

    serviceTime = static_cast<unsigned int>((argv[0]) ? atoi(argv[0]) : 100);
    period = static_cast<unsigned int>((argv[1]) ? atoi(argv[1]) : 200);
    maxSize = static_cast<unsigned int>((argv[2]) ? atoi(argv[2]) : 6);

    return 0;
}

void *start_terminal(void *arg) {
    const int SEM_ID = *(int *) arg;
    int fuel_type = 3; //TODO
    int semid, shmid;
    Car *myCar;
    std::list<Car>* list;

    if ((semid = semget(SEM_ID, 1, PERMS | IPC_CREAT)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(list), PERMS | IPC_CREAT)) < 0) {
        sys_err("server: can not get shared memory segment");
    }
    if ((list = (std::list<Car>*) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("server: shared memory attach error");
    }

    semctl(semid, 0, SETVAL, 0);
    while (true) {
        if (semctl(semid, 0, GETVAL, 0))
            continue;

        semctl(semid, 0, SETVAL, 1);

        /*TODO: Реализовать соответствующую логику обработки машины
         * Использовать sleep()*/
        for(Car car: *list){
            if (car.fuel_type == fuel_type){
                myCar = &car;
                break;
            }
        }

        sleep(serviceTime);


        semctl(semid, 0, SETVAL, 0);
        if (true) break;
    }

    if (semctl(semid, 0, IPC_RMID, (struct semid_ds *) 0) < 0) {
        sys_err("server: semaphore remove error");
    }

    shmdt(list);
    if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) nullptr) < 0) {
        sys_err("server: shared memory remove error");
    }
    exit(0);
}

void *start_cars_flow(void *arg) {

    int shmid;
    std::list<Car>* list;

    if ((shmid = shmget(QUEUE, sizeof(list), 0)) < 0) {
        sys_err("client: can not get shared memory segment");
    }

    if ((list = (std::list<Car> *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("client: shared memory attach error");
    }

    for (int i = 0; i < 150; i++) {
        Car car;
        car.car_id = i;
        car.fuel_type = 2;
        if(list->size() < maxSize){
            list->push_back(car);
        }
        sleep(period);
    }
    shmdt(list);
    exit(0);
}

void sys_err(const char *msg) {
    puts(msg);
    exit(1);
}
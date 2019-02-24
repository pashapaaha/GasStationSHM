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
#include <vector>

using namespace std;

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


void init(int, char **);

void *start_terminal(void *);

void *start_cars_flow(void *);

void sys_err(const char *);

unsigned int serviceTime;
unsigned int period;
unsigned int maxSize;

int main(int argc, char **argv) {
    init(argc, argv);
    cout << serviceTime << " " << period << " " << maxSize << endl;

    return 0;
}

void init(int argc, char **argv) {

    serviceTime = 100;
    period = 200;
    maxSize = 6;

    if (argc > 1) {
        serviceTime = static_cast<unsigned int>(atoi(argv[1]));
    } else {
        return;
    }
    if (argc > 2) {
        period = static_cast<unsigned int>(atoi(argv[2]));
    } else {
        return;
    }
    if(argc > 3) {
        maxSize = static_cast<unsigned int>(atoi(argv[3]));
    } else {
        return;
    }
}

void *start_terminal(void *arg) {
    const int SEM_ID = *(int *) arg;
    int fuel_type = 3; //TODO
    int semid, shmid;
    Car *myCar;
    vector<Car>* cars;

    if ((semid = semget(SEM_ID, 1, PERMS | IPC_CREAT)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(cars), PERMS | IPC_CREAT)) < 0) {
        sys_err("server: can not get shared memory segment");
    }
    if ((cars = (vector<Car> *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("server: shared memory attach error");
    }

    semctl(semid, 0, SETVAL, 0);
    while (true) {
        if (semctl(semid, 0, GETVAL, 0))
            continue;

        semctl(semid, 0, SETVAL, 1);

        for (Car car: *cars) {
            if (car.fuel_type == fuel_type) {
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

    shmdt(cars);
    if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) nullptr) < 0) {
        sys_err("server: shared memory remove error");
    }
    exit(0);
}

void *start_cars_flow(void *arg) {

    int shmid;
    void* tempPointer;
    vector<Car>* cars;

    if ((shmid = shmget(QUEUE, sizeof(cars), 0)) < 0) { // second arg???
        sys_err("client: can not get shared memory segment");
    }

    if ((tempPointer = (list<Car> *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("client: shared memory attach error");
    }
    cars = new(tempPointer) vector<Car>;

    for (int i = 0; i < 150; i++) {
        Car car;
        car.car_id = i;
        car.fuel_type = 2;//TODO: get random
        if (cars->size() < maxSize) {
            cars->push_back(car);
        }
        sleep(period);
    }
    shmdt(cars);
    exit(0);
}

void sys_err(const char *msg) {
    puts(msg);
    exit(1);
}
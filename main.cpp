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
#include <time.h>

using namespace std;

#define QUEUE 2000
#define SEM 2001

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
bool flowClosed;
bool queueIsEmpty;

int main(int argc, char **argv) {
    init(argc, argv);
    cout << serviceTime << " " << period << " " << maxSize << endl;

    pthread_t queue;
    int result;

    result = pthread_create(&queue, nullptr, start_cars_flow, nullptr);
    if (result != 0) {
        perror("first thread creating");
        return EXIT_FAILURE;
    }

    result = pthread_join(queue, nullptr);
    if (result != 0) {
        perror("waiting for a first thread");
        return EXIT_FAILURE;
    }

    int semid;

    if ((semid = semget(SEM, 1, PERMS | IPC_CREAT)) < 0) {
        sys_err("terminal : can not get semaphore");
    }


    return 0;
}

void init(int argc, char **argv) {

    srand(time(nullptr));

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
    if (argc > 3) {
        maxSize = static_cast<unsigned int>(atoi(argv[3]));
    } else {
        return;
    }
}

void *start_terminal(void *arg) {
    const int fuel_type = *(int *) arg;
    int semid, shmid;
    Car *cars;

    if ((semid = semget(SEM, 1, 0)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(Car) * maxSize, 0)) < 0) {
        sys_err("server: can not get shared memory segment");
    }
    if ((cars = (Car *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("server: shared memory attach error");
    }


    semctl(semid, 0, SETVAL, 0);

    while (true) {
        label:
        if (semctl(semid, 0, GETVAL, 0))
            continue;

        semctl(semid, 0, SETVAL, 1);

        for (int i = 0; i < maxSize; i++) {
            if (cars[i].fuel_type == fuel_type) {
                cars[i] = nullptr;//TODO:
                break;
            }
        }

        semctl(semid, 0, SETVAL, 0);

        sleep(serviceTime);

        if (flowClosed) {
            for (int i = 0; i < maxSize; i++) {
                if (cars[i] != nullptr) {
                    goto label;
                }
            }

            if (!queueIsEmpty && shmctl(shmid, IPC_RMID, (struct shmid_ds *) nullptr) < 0) {
                queueIsEmpty = true;
                sys_err("server: shared memory remove error");
            }
            break;
        }
    }

    shmdt(cars);

    exit(0);
}

void *start_cars_flow(void *arg) {

    printf("start cars flow\n");

    int shmid, semid;
    Car *cars; //TODO:

    if ((semid = semget(SEM, 1, 0)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(Car) * maxSize, PERMS | IPC_CREAT)) < 0) {
        sys_err("client: can not get shared memory segment");
    }

    if ((cars = (Car *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("client: shared memory attach error");
    }

    Car *c = new(cars) Car[maxSize];

    for (int i = 0; i < 150; i++) {
        while (semctl(semid, 0, GETVAL, 0));
        semctl(semid, 0, SETVAL, 1);
        Car car;
        car.car_id = i;
        car.fuel_type = rand() % 4;
        for (int j = 0; j < maxSize; j++) {
            if (cars[j] == nullptr) {
                cars[j] = &car; //TODO:
                printf("car #%d with fuel type #%d is gas station", cars[j]->car_id, cars[j]->fuel_type);
                break;
            }
        }
        semctl(semid, 0, SETVAL, 0);
        sleep(period);
    }
    flowClosed = true;
    shmdt(cars);
    exit(0);
}

void sys_err(const char *msg) {
    puts(msg);
    exit(1);
}
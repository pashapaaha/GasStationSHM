#include "resources.h"


unsigned int serviceTime;
unsigned int period;
unsigned int maxSize;
bool flowClosed;
bool queueIsEmpty;

int index = 1;

const int TERMINALS_COUNT = 5;

int main(int argc, char **argv) {
    init(argc, argv);
    cout << serviceTime << " " << period << " " << maxSize << endl;

    pthread_t queue;
    pthread_t terminals[TERMINALS_COUNT];
    int result;

    if ((result = semget(SEM, 1, PERMS | IPC_CREAT)) < 0) {
        sys_err("terminal : can not get semaphore");
    }


    result = pthread_create(&queue, nullptr, start_cars_flow, nullptr);
    if (result != 0) {
        perror("first thread creating");
        return EXIT_FAILURE;
    }

    int types[TERMINALS_COUNT] = {0, 0, 1, 1, 2};

    sleep(2);
    for(int i = 0; i < TERMINALS_COUNT; i++){
        result = pthread_create(&terminals[i], nullptr, start_terminal, &types[i]);
        if (result != 0) {
            perror("first thread creating");
            return EXIT_FAILURE;
        }
    }

    result = pthread_join(queue, nullptr);
    if (result != 0) {
        perror("waiting for a first thread");
        return EXIT_FAILURE;
    }



    return 0;
}

void init(int argc, char **argv) {

    srand(time(nullptr));

    serviceTime = 3;
    period = 4;
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
    const int fuel_type = *(int*) arg;
    printf("Fuel type is %s \n", get_fuel_name(fuel_type));
    int id = index++;

    char filename[20];
    sprintf(filename, "%d%s%s%s", id, "-terminal-", get_fuel_name(fuel_type), ".txt");
    ofstream fout(filename);

    fout << "Terminal #" << id << " starts work" << endl;

    int semid, shmid;
    id_and_type *cars;

    if ((semid = semget(SEM, 1, 0)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(id_and_type) * maxSize, 0)) < 0) {
        sys_err("server: can not get shared memory segment");
    }
    if ((cars = (id_and_type *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("server: shared memory attach error");
    }


    while (true) {
        label:
        bool needService = false;
        if (semctl(semid, 0, GETVAL, 0))
            continue;

        semctl(semid, 0, SETVAL, 1);

        for (int i = 0; i < maxSize; i++) {
            if (cars[i].fuel_type == fuel_type && cars[i].id != 0) {
//                printf("Terminal serviced car #%d with fuel %s\n", cars[i].id, get_fuel_name(cars[i].fuel_type));
                fout << " - Terminal " << id << ": Car #" << cars[i].id << "with fuel" << get_fuel_name(cars[i].fuel_type) << endl;
                cars[i].id = 0;
                needService = true;
                break;
            }
        }

        semctl(semid, 0, SETVAL, 0);

        if(needService) {
            sleep(serviceTime);
        }

        if (flowClosed) {
            for (int i = 0; i < maxSize; i++) {
                if (!cars[i].id) {
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
    fout.close();
    exit(0);
}

void *start_cars_flow(void *arg) {

    printf("start cars flow\n");

    int shmid, semid;
    id_and_type *cars;

    ofstream fout("car_flow.txt");
    fout << "start cars flow" << endl;

    if ((semid = semget(SEM, 1, 0)) < 0) {
        sys_err("terminal : can not get semaphore");
    }
    if ((shmid = shmget(QUEUE, sizeof(id_and_type*) * maxSize, PERMS | IPC_CREAT)) < 0) {
        sys_err("client: can not get shared memory segment");
    }

    if ((cars = (id_and_type *) shmat(shmid, nullptr, 0)) == nullptr) {
        sys_err("client: shared memory attach error");
    }

    semctl(semid, 0, SETVAL, 0);


    for (int i = 1; i < 151; i++) {
        while (semctl(semid, 0, GETVAL, 0));
        semctl(semid, 0, SETVAL, 1);
        id_and_type car;
        car.id = i;
        car.fuel_type = rand() % 3;
        for (int j = 0; j < maxSize; j++) {
            if (!cars[j].id) {
                cars[j] = car;
//                printf("car #%d with fuel type %s is gas station\n", cars[j].id, get_fuel_name(cars[j].fuel_type));
                fout << " - Car #" << cars[j].id << " with fuel type  " << get_fuel_name(cars[j].fuel_type) << endl;
                break;
            }
        }
        semctl(semid, 0, SETVAL, 0);
        sleep(period);
    }
    flowClosed = true;
    fout.close();
    shmdt(cars);
    exit(0);
}

void sys_err(const char *msg) {
    puts(msg);
    exit(1);
}

char* get_fuel_name(int number){
    switch (number) {
        case 0:
            return "AI-76";
        case 1:
            return "AI-92";
        case 2:
            return "AI-95";
        default:
            return "";
    }
}
//
// Created by panda on 04.03.19.
//

#ifndef GASSTATION_RESORCES_H
#define GASSTATION_RESORCES_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>

using namespace std;

#define QUEUE 2000
#define SEM 2001

#define PERMS 0666

typedef struct {
    int id;
    int fuel_type;
} Car;

void init(int, char **);

void *start_terminal(void *);

void *start_cars_flow(void *);

void sys_err(const char *);

char* get_fuel_name(int);

void sort_queue(Car*);

void print_queue(Car*);

#endif //GASSTATION_RESORCES_H

// example demo code for locks
// S.W. Smith, CS58, Dartmouth College


// Remember to compile with the "gcc -lpthread -m32" options


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>   // for threads
#include <semaphore.h> // for semaphores

#include <unistd.h>

#define INPUT_LEN 1024
#define NORWICH 0
#define BRIDGE 1
#define HANOVER 2
#define SCALE 100000

#  define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP \
  { { 0, 0, 0, PTHREAD_MUTEX_ERRORCHECK_NP, 0, { 0 } } }

typedef struct CAR {
    int dir;
    int id;
    int loc;
} car_t;

pthread_cond_t bridgeCondition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock =  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
int maxCar, n2h, h2n, flow = 0, car = 0, totalCars, cross = 0;
car_t* cars;

void printOutsideLock() {
    int rc = pthread_mutex_lock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
    
    // Printing
    char hanover[INPUT_LEN] = "";
    char norwich[INPUT_LEN] = "";
    char bridge[INPUT_LEN] = "";
    char item[INPUT_LEN];
    int i, hCnt = 0, nCnt = 0, bCnt = 0;
    for(i = 0; i < INPUT_LEN; i++) {
        switch(cars[i].loc) {
            case NORWICH:
                nCnt++;
                sprintf(item, "(%d)", cars[i].id);
                strcat(norwich, item);
                break;
            case HANOVER:
                hCnt++;
                sprintf(item, "(%d)", cars[i].id);
                strcat(item, hanover);
                strcpy(hanover, item);
                break;
            case BRIDGE:
                bCnt++;
                sprintf(item, "(%s%d%s)",  cars[i].dir == -1 ? "<-" : "", cars[i].id, cars[i].dir == -1 ? "" : "->");
                strcat(bridge, item);
                break;
            default:
                break;
        }
    }
    char bEmpty[INPUT_LEN];
    if(bCnt == 0) {
        strcpy(bEmpty, "\tBridge is empty!");
    }
    while(bCnt < maxCar) {
        if(flow == -1) {
            char tmp[INPUT_LEN];
            strcpy(tmp, "      ");
            strcat(tmp, bridge);
            strcpy(bridge, tmp);
        } else {
            strcat(bridge, "      ");
        }
        bCnt++;
    }
    strcat(norwich, "");
    strcat(hanover, "");
    strcat(bridge, "");
    printf("%d [%s] %d%s\n", nCnt, bridge, hCnt, bEmpty);
    //printf("%s\n%s\n%s\n", norwich, bridge, hanover);

    // Checking stats
    if(car > maxCar) {
        fprintf(stderr, "#Cars on bridge %d overflow %d\n", car, maxCar);
    } 
    if(car < 0) {
        fprintf(stderr, "#Cars on bridge %d is negative\n", car);
    }
    
    // Release the lock
    rc = pthread_mutex_unlock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
}

int safeGo(int dir) {
    // Read and perceive the locked resource       
    if(car == 0 && dir != flow) {
        return 1;
    } else if(car < maxCar && flow == dir) {
        return 1;
    }
    return 0;
}

void exitBridge(car_t* aCar) {
    // Lock the share resource
    int rc = pthread_mutex_lock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }

    // Update condition and signal for anyone that might be waiting
    car--;
    pthread_cond_broadcast(&bridgeCondition);

    // Release the lock
    rc = pthread_mutex_unlock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
    // printOutsideLock();
}

void onBridge(car_t* aCar) {
    usleep(SCALE * cross);
    aCar->loc += aCar->dir * 10; // in order to step way out of the bridge
    printOutsideLock();
}

void arriveBridge(car_t* aCar) {
    int rc;

    // Lock the share resource
    rc = pthread_mutex_lock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }

    // Wait if necessary
    while(!safeGo(aCar->dir)) {
        pthread_cond_wait(&bridgeCondition, &lock);
    }

    // Write the locked resource if necessary
    flow = aCar->dir;
    car++;
    aCar->loc += aCar->dir;
    
    // Release the lock
    rc = pthread_mutex_unlock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
    
    printOutsideLock();
}

void* oneVehicle(car_t* aCar) {
    arriveBridge(aCar);
    onBridge(aCar);
    exitBridge(aCar);
}


int main(int argc, char *argv[]) {
    int rc, i; 
    if(argc != 4) {
        printf("brdige usage: ./bridge <#maxCar on the bridge> <car prob to Hanover (0~100)> <car prob to Norwich (0~100)>\n");
        return;
    } else {
        maxCar = atoi(argv[1]);
        n2h = atoi(argv[2]);
        cross = 2;
        h2n = atoi(argv[3]);
    }
    cars = (car_t*) malloc(sizeof(car_t) * INPUT_LEN);
    for(i = 0; i < INPUT_LEN; i++) {
        cars[i].loc = -1;
    }

    // Start the children threads
    srand(time(NULL));
    i = 0;
    while(1) {
        if((rand() % 100) < n2h ) {
            cars[i % INPUT_LEN].loc = NORWICH;
            cars[i % INPUT_LEN].id = i;
            cars[i % INPUT_LEN].dir = 1;
            pthread_t* carThread = (pthread_t*) malloc(sizeof(pthread_t));
            pthread_create(carThread, NULL, oneVehicle, cars + (i % INPUT_LEN));
            i++;
        }
        if((rand() % 100) < h2n ) {
            cars[i % INPUT_LEN].loc = HANOVER;
            cars[i % INPUT_LEN].id = i;
            cars[i % INPUT_LEN].dir = -1;
            pthread_t* carThread = (pthread_t*) malloc(sizeof(pthread_t));
            pthread_create(carThread, NULL, oneVehicle, cars + (i % INPUT_LEN));
            i++;
        }
        usleep(SCALE);
    }
    return 0;
}
  





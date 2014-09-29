// example demo code for locks
// S.W. Smith, CS58, Dartmouth College


// Remember to compile with the "gcc -lpthread -m32" options


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>   // for threads
#include <semaphore.h> // for semaphores

#include <unistd.h>

#define INPUT_LEN 1024
#define NORWICH 0
#define BRIDGE 1
#define HANOVER 2

#  define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP \
  { { 0, 0, 0, PTHREAD_MUTEX_ERRORCHECK_NP, 0, { 0 } } }

typedef struct CAR {
    int dir;
    int id;
    int loc;
} car_t;

pthread_mutex_t lock =  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
int maxCar, n2h, h2n, flow = 0, car = 0, totalCars;
car_t* cars;

void printOutsideLock() {
    int rc = pthread_mutex_lock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
    
    // Checking stats
    if(car > maxCar) {
        fprintf(stderr, "#Cars on bridge %d overflow %d\n", car, maxCar);
    } 
    if(car < 0) {
        fprintf(stderr, "#Cars on bridge %d is negative\n", car);
    }
    if(car < maxCar) {
        if(flow > 0 && n2h > 0) {
            fprintf(stderr, "#Cars on bridge %d < %d, with %d remaining cars waiting to go to Hanvoer\n", car, maxCar, n2h);
        }
        if(flow < 0 && h2n > 0) {
            fprintf(stderr, "#Cars on bridge %d < %d, with %d remaining cars waiting to go to Norwich\n", car, maxCar, h2n);
        }
    }
    
    // Printing
    char hanover[INPUT_LEN] = "[Hanover: ";
    char norwich[INPUT_LEN] = "[Norwich: ";
    char bridge[INPUT_LEN] = "[Bridge: ";
    char item[INPUT_LEN];
    int i;
    for(i = 0; i < totalCars; i++) {
        sprintf(item, "(%d,%s)", cars[i].id, cars[i].dir == 1 ? "->" : "<-");
        switch(cars[i].loc) {
            case NORWICH:
                strcat(norwich, item);
                break;
            case HANOVER:
                strcat(hanover, item);
                break;
            case BRIDGE:
                strcat(bridge, item);
                break;
            default:
                break;
        }
    }
    strcat(norwich, "]");
    strcat(hanover, "]");
    strcat(bridge, "]");
    printf("%s %s %s\n", norwich, bridge, hanover);

    // Release the lock
    rc = pthread_mutex_unlock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
}

void deQueueWithinLock(car_t* aCar) {
    if(aCar->dir > 0) {
        n2h--;
    } else {
        h2n--;
    }
}

void exitBridge(car_t* aCar) {
    // Lock the share resource
    int rc = pthread_mutex_lock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }

    // Update stats
    car--;
    aCar->loc += aCar->dir;

    // Release the lock
    rc = pthread_mutex_unlock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
    printOutsideLock();
}

void onBridge(car_t* aCar) {
    sleep(1);
    aCar->loc += aCar->dir;
    printOutsideLock();
}

void arriveBridge(car_t* aCar) {
    int rc, enterTheBridge = 0;
    printOutsideLock();

    while(!enterTheBridge) {
        // Lock the share resource
        rc = pthread_mutex_lock(&lock);
        if(rc) {
            fprintf(stderr, "Lock acquire fails.\n");
            exit(-1);
        }

        // Read and perceive the locked resource       
        if(car == 0) {
            enterTheBridge = 1;
        } else if(car <= maxCar) {
            if(flow == aCar->dir) {
                enterTheBridge = 1;
            }
        }

        // Write the locked resource if necessary
        if(enterTheBridge) {
            deQueueWithinLock(aCar);
            flow = aCar->dir;
            car++;
            aCar->loc += aCar->dir;
        }

        // Release the lock
        rc = pthread_mutex_unlock(&lock);
        if(rc) {
            fprintf(stderr, "Lock acquire fails.\n");
            exit(-1);
        }
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
        printf("brdige usage: ./bridge <#maxCar on the bridge> <#cars to Hanover> <#car to Norwich>\n");
        return;
    } else {
        maxCar = atoi(argv[1]);
        n2h = atoi(argv[2]);
        h2n = atoi(argv[3]);
    }
    totalCars = n2h + h2n;
    pthread_t* carThreads = (pthread_t*) malloc(sizeof(pthread_t) * totalCars);
    cars = (car_t*) malloc(sizeof(car_t) * totalCars);

    // Start the children threads
    for(i = 0; i < totalCars; i++) {
        cars[i].id = i;
        if(i < n2h) {
            cars[i].dir = 1;
            cars[i].loc = NORWICH;
            rc = pthread_create(&carThreads[i], NULL, oneVehicle, cars + i);
        } else {
            cars[i].dir = -1;
            cars[i].loc = HANOVER;
            rc = pthread_create(&carThreads[i], NULL, oneVehicle, cars + i);
        }
    }

    // Wait till the end
    for(i = 0; i < totalCars; i++) {
        rc = pthread_join(carThreads[i], NULL);
        printf("%dth child is done\n", i);
    }
    printf("Simulation complete.\n");
    return 0;
}
  





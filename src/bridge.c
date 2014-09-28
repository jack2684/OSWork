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


#define KIDS  4

#  define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP \
  { { 0, 0, 0, PTHREAD_MUTEX_ERRORCHECK_NP, 0, { 0 } } }


pthread_mutex_t lock =  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
int maxCar, n2h, h2n, flow = 0, car = 0;

void deQueueWithinLock(int direction) {
    if(direction > 0) {
        n2h--;
    } else {
        h2n++;
    }
}

void exitBridge(int direction) {
    // Lock the share resource
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

    // Update stats
    car--;

    // Release the lock
    rc = pthread_mutex_unlock(&lock);
    if(rc) {
        fprintf(stderr, "Lock acquire fails.\n");
        exit(-1);
    }
}

void onBridge(int direction) {
   sleep(1);
}

void arriveBridge(int direction) {
    int rc, enterTheBridge = 0;
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
            if(flow == direction) {
                enterTheBridge = 1;
            }
        }

        // Write the locked resource if necessary
        if(enterTheBridge) {
            deQueueWithinLock(direction);
            flow = direction;
        }

        // Release the lock
        rc = pthread_mutex_unlock(&lock);
        if(rc) {
            fprintf(stderr, "Lock acquire fails.\n");
            exit(-1);
        }
    }

}

void oneVehicle(int direction) {
    arriveBridge(direction);
    onBridge(direction);
    exitBridge(direction);
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
    pthread_t* carThreads = (pthread_t*) malloc(sizeof(pthread_t) * (n2h + h2n));

    // Start the children threads
    for(i = 0; i < n2h + h2n; i++) {
        if(i < n2h) {
            rc = pthread_create(&carThreads[i], NULL, oneVehicle, 1);
        } else {
            rc = pthread_create(&carThreads[i], NULL, oneVehicle, -1);
        }
    }

    // Wait till the end
    for(i = 0; i < n2h + h2n; i++) {
        printf("Waiting for %dth child", i);
        rc = pthread_join(carThreads[i], NULL);
    }

    printf("Simulation complete.\n");
    return 0;
}
  





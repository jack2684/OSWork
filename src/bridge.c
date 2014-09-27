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


// for pretty printing
char *spaces(int kid) {
  switch (kid) {
  case 0:
    return ":";
  case 1:
    return "--";
  case 2:
    return "++++";
  case 3:
    return "======";
  }

  return "********";
	
}  


void *lockfun(void *vargp) {


  int me = (int) vargp;
  int rc;


  printf("%s Thread %d wants to work on the complex data structure.\n", spaces(me), me);
  printf("%s Thread %d: trying to acquire lock...\n\n",spaces(me), me);

  rc = pthread_mutex_lock(&lock);
  if (rc) {
    printf("Thread %d: acquire failed!\n", me);
    exit(-1);
  }
  printf("\n%s Thread %d: acquired the lock!\n\n", spaces(me), me);  

  printf("%s Thread %d: In critical section.  Now I can work on the data structure and temporarily make the invariant false!\n", spaces(me), me);

  sleep(1);

  printf("%s (Thread %d: pretending to work on the data)\n", spaces(me), me);

  sleep(1);

  printf("%s Thread %d: All done.  I've restored the invariant. About to release lock...\n", spaces(me), me);
  rc = pthread_mutex_unlock(&lock);
  if (rc) {
    printf("Thread %d: release failed!\n", me);
    exit(-1);
  }

  
  printf("%s Thread %d: exiting.\n",spaces(me), me);

  return NULL;
}




//==================================

int
main(int argc, char *argv[]) {
    if(argc <= 1) {
        printf("brdige usage: ./bridge <#cars to Hanover> <#car to Norwich>")
    }


  pthread_t kids[KIDS];

  int i, rc;
  void *(*kidfun)(void *);




  for (i = 0; i < KIDS; i++) {
    
    rc = pthread_create(&kids[i],  // thread data structure to be written
			NULL,    // attributes (we'll ignore)
			lockfun, // the function to be run 
                        (void *) i); // the argument to the function

    if (rc) {
      printf("pthread_create failed!\n");
      exit(-1);
    }

  }


  for (i = 0; i < KIDS; i++) {

    rc = pthread_join(kids[i],  // thread to wait for
		      NULL);   // where to save the rc from pthread_exit(), 
                             // (if we care. for this example, we don't)

  }


  return 0;
}
  





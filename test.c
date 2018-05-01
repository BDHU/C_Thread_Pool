// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <string.h>
#include "thread.h"


int mutex_flag;
const int prime = 4222234741;
const char* data = "A purely peer-to-peer version of electronic cash would allow online\
payments to be sent directly from one party to another without going through a\
financial institution. Digital signatures provide part of the solution, but the main\
benefits are lost if a trusted third party is still required to prevent double-spending.\
We propose a solution to the double-spending problem using a peer-to-peer network.\
The network timestamps transactions by hashing them into an ongoing chain of\
hash-based proof-of-work, forming a record that cannot be changed without redoing\
the proof-of-work. The longest chain not only serves as proof of the sequence of\
events witnessed, but proof that it came from the largest pool of CPU power. As\
long as a majority of CPU power is controlled by nodes that are not cooperating to\
attack the network, they'll generate the longest chain and outpace attackers. The\
network itself requires minimal structure. Messages are broadcast on a best effort\
basis, and nodes can leave and rejoin the network at will, accepting the longest\
proof-of-work chain as proof of what happened while they were gone. \n";

void short_task(void* arg);
void long_task(void* arg);
void test_fib_serires(void *num_ptr);
void test1(void* arg) {
  for(int i=0; i<10000; i++);
}

int main(int argc, char** argv) {
  int o;
  int workers = 0;
  int test_size = 1000;
  // worker can be predefined or set to default
  struct option opts[3] = {
    {"workers", required_argument, NULL, 'w'},
    {"mutex", no_argument, &mutex_flag, 1},
    { NULL, 0, NULL, 0}
  };

  // parse arguments
  while ((o = getopt_long_only(argc, argv, "w:", opts, NULL)) != -1 ) {
    switch (o) {
      case 0:
      	break;
      case 'w':
        workers = atoi(optarg);
        break;
      default:
        printf("default case, don't recognize anything %d \n", o);
    }
  }

  thread_pool_init(workers, mutex_flag);
  int results[test_size];
  for (int i=0; i<test_size; i++) {
    results[i] = i;
  }
  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  
  for (int i=0; i<test_size; i++) {
    thread_pool_add(short_task, results+i);  
    thread_pool_add(long_task, (void*) i);
  }
  thread_pool_wait();
  gettimeofday(&t2, NULL);
  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

  // for (int i=0; i<test_size; i++) {
  //   printf("%d ", results[i]);
  // }

  printf("done waiting for jobs: execution time: %0.5gms\n", elapsedTime);
}

/* =================== user defined function =================== */
void test_fib_serires(void * n) {
  int i = 0;
  int j = 1;
  int num = (int) n;
  int index = 0;
  for (; index<num; index++) {
    int temp = j;
    j = j + i;
    i = temp;
  }
}

/* ========================= */
void* spawn_child_threads_helper_func(void *aux) {
  printf("Thread id: %lu\n", pthread_self());
  return NULL;
}

void test_spawn_child_threads(void *aux) {
  int i = 0;
  pthread_t threads[10];
  for (; i<10; i++) {
    pthread_create(&threads[i], NULL, spawn_child_threads_helper_func, NULL);
  }
}
/* ========================= */

void test_recursion(void *a) {
  int *aux = (int *)a;
  int num = *aux;
  if (num == 0)
    return;
  (*aux) = (*aux) - 1;
  test_recursion(aux);
}

void test_mutex(void *m) {
  pthread_mutex_lock(m);
  printf("print in mutex\n");
  pthread_mutex_unlock(m);
}

void test_spinlock(void *spin) {
  pthread_spin_lock(spin);
  printf("print in spinlock\n");
  pthread_spin_unlock((pthread_spinlock_t *)spin);
}

void test_async_print(void *num) {
  int i = 0;
  for (; i<*((int *)num); i++) {
    printf("num: %d\n", *((int *)num));
  }
}

/* spawn enormous amout of tasks */
void test_spawn_tasks(void *num) {
  thread_pool_init(*((int *)num), mutex_flag);
}

void test_redundant_thread_pool_init(void *num) {
  int i = 0;
  for (; i<*((int *)num); i++) {
    thread_pool_init(*((int *)num), mutex_flag);
  }
}

// ============== experiment ==================

void long_task(void* arg) {
  char buf[30];
  snprintf(buf, 30, "output/tmp-%d", (int)arg);
  
  FILE *f = fopen(buf, "w+");
  if (!f) {
    printf("Failed to create file \n");
    return;
  }

  size_t size = strlen(data)+1;
  size_t wsize = fwrite(data, 1, size, f);
  if (wsize != size)
    printf("Failed to write %lu bytes, only wrote %lu \n", size, wsize);
} 

// compute sum and then hash it
void short_task(void* arg) {
  int sum = 0;
  int* p = (int*) arg;
  // compute sum 
  for (int x=1; x<*p; x++) 
    sum += x;

  *p = (sum * (sum-2)) % prime;
}

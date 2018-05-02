// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include "thread.h"
#include "shared-test.c"

int mutex_flag;

void test_fib_serires(void *num_ptr);
void test1(void* arg) {
  for(int i=0; i<10000; i++);
}

int main(int argc, char** argv) {
  int c;
  int workers = 0;
  int test_size = 1000;
  // worker can be predefined or set to default
  struct option opts[3] = {
    {"workers", required_argument, NULL, 'w'},
    {"mutex", no_argument, &mutex_flag, 1},
    { NULL, 0, NULL, 0}
  };

  // parse arguments
  while ((c = getopt_long_only(argc, argv, "w:", opts, NULL)) != -1 ) {
    switch (c) {
      case 0:
      	break;
      case 'w':
        workers = atoi(optarg);
        break;
      default:
        printf("default case, don't recognize anything %d \n", c);
    }
  }

  thread_pool_init(workers, mutex_flag);
  int results[test_size];
  for (int i=0; i<test_size; i++) {
    results[i] = i;
  }

  int lnum = 0;
  int snum = 0;
 
  struct out o[lnum_limit];
  for (int i=0; i<lnum_limit; i++) {
    o[i].dir = "output";
    o[i].arg = i;
  }
  // set rand
  srand(0);
  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  
  for (int i=0; i<test_size; i++) {
    int x = rand() % 100;
    if (x<rate) {
      if (snum < snum_limit) {
        thread_pool_add(short_task, results+snum);  
        snum++;        
      }
    } else {
      if (lnum < lnum_limit) {
        thread_pool_add(long_task, o+lnum);      
        lnum++; 
      }
    }
  }

  for (int i=snum; i<snum_limit; i++)
    thread_pool_add(short_task, results+i); 
  for (int i=lnum; i<lnum_limit; i++) 
    thread_pool_add(long_task, o+i);

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

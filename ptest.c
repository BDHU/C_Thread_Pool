// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include "shared-test.c"

int mutex_flag;
// void* worker_func(void* arg);

// struct work_load {
//   pthread_t tid;
//   int* start;
//   int size;
//   int lstart;
// };

int main(int argc, char** argv) {
  int test_size = 1000;
  
  srand(0);

  int lnum = 0;
  int snum = 0;

  // worker version
  // struct work_load work[test_size];
  pthread_t tids1[snum_limit];
  pthread_t tids2[lnum_limit];
  int results[snum_limit];
  for (int i=0; i<snum_limit; i++) {
    results[i] = i;
  }

  struct out o[lnum_limit];
  for (int i=0; i<lnum_limit; i++) {
    o[i].dir = "poutput";
    o[i].arg = i;
  }

  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  
  
  for (int i=0; i<test_size; i++) {
    int x = rand() % 100;
    if (x<rate) {
      if (snum < snum_limit) {
        if (pthread_create(&tids1[snum], NULL, short_task, results+snum) != 0) {
          printf("failed to create thread %d \n", i);
          exit(1);
        }
        snum++;        
      }
    } else {
      if (lnum < lnum_limit) {
        if (pthread_create(&tids2[lnum], NULL, long_task, o+lnum) != 0) {
          printf("failed to create thread %d \n", i);
          exit(1);
        }
        lnum++; 
      }
    }
  }

  for (int i=snum; i<snum_limit; i++) {
    if (pthread_create(&tids1[i], NULL, short_task, results+i) != 0) {
      printf("failed to create thread %d \n", i);
      exit(1);
    }
  }

  for (int i=lnum; i<lnum_limit; i++) {
    if (pthread_create(&tids2[i], NULL, long_task, o+i) != 0) {
      printf("failed to create thread %d \n", i);
      exit(1);
    }
  }
  
  for (int i=0; i<snum_limit; i++) {
    if (pthread_join(tids1[i], NULL) != 0) {
      printf("failed to wait thread %d \n", i);
    }
  }

  for (int i=0; i<lnum_limit; i++) {
    if (pthread_join(tids2[i], NULL) != 0) {
      printf("failed to wait thread %d \n", i);
    }
  }

  gettimeofday(&t2, NULL);
  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("done waiting for jobs: execution time: %0.5gms\n", elapsedTime);
}

// ============== experiment ==================

// void* worker_func(void* arg) {
//   struct work_load* wl = (struct work_load*)arg;
//   struct out o;
//   o.dir = "poutput";
//   o.arg = wl->lstart;
//   short_task(wl->start);
//   long_task(&o);
//   return NULL;
// } 
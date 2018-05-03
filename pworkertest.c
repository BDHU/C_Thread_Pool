// test program
#define _GNU_SOURCE

#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include "shared-test.c"

int mutex_flag;
typedef void* tfunc(void* aux);
struct work {
  tfunc* func; 
  void* aux;
};

struct work_load {
  pthread_t tid;
  int start;
  int end;
};

// preallocate this
struct work jobs[1000];
void* worker_func(void* arg);

int main(int argc, char** argv) {
  int c;
  int workers = 0;
  int test_size = 1000;  
  char test_type[10];

  // worker can be predefined or set to default
  struct option opts[4] = {
    {"workers", required_argument, NULL, 'w'},
    {"mutex", no_argument, &mutex_flag, 1},
    {"test", required_argument, NULL, 't'},
    { NULL, 0, NULL, 0}
  };

  // parse arguments
  while ((c = getopt_long_only(argc, argv, "wt:", opts, NULL)) != -1 ) {
    switch (c) {
      case 0:
      	break;
      case 'w':
        workers = atoi(optarg);
        break;
      case 't':
        strcpy(test_type, optarg);
        break;
      default:
        printf("default case, don't recognize anything %d \n", c);
    }
  }

  // worker version
  workers = workers == 0 ? get_nprocs() : workers;
  printf("Running with %d threads \n", workers);  
  
  int lnum = 0;
  int snum = 0;
  struct out o[lnum_limit];
  for (int i=0; i<lnum_limit; i++) {
    o[i].dir = "pwoutput";
    o[i].arg = i;
  }

  struct work_load work[workers];

  int results[snum_limit];
  for (int i=0; i<snum_limit; i++) {
    results[i] = i;
  }
  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  

  // populate works.
  for (int i=0; i<test_size; i++) {
    if (lnum >= lnum_limit) {
      jobs[i].func = short_task;
      jobs[i].aux = results+snum;  
      snum++;  
      continue;
    }
    if (snum >= snum_limit) {
      jobs[i].func = long_task;
      jobs[i].aux = o+lnum;
      lnum++; 
      continue;
    }

    int x = rand() % 100;
    if (x<rate) {
      jobs[i].func = short_task;
      jobs[i].aux = results+snum;  
      snum++;     
    } else {
      jobs[i].func = long_task;
      jobs[i].aux = o+lnum;
      lnum++; 
    }
  }

  int work_assigned = 0;
  int average_load = test_size / workers;

  cpu_set_t cpuset[workers];
  for (int i=0; i<workers; i++) {
    CPU_ZERO(&cpuset[i]);    
    work[i].start = work_assigned;
    work_assigned += average_load;
    work[i].end = work_assigned;
    if (i == workers - 1) {
      work[i].end = test_size;
    } 

    if (pthread_create(&work[i].tid, NULL, worker_func, work+i) != 0) {
      printf("failed to create thread %d \n", i);
    }
    CPU_SET(i, &cpuset[i]);    
    pthread_setaffinity_np(work[i].tid, sizeof(cpu_set_t), &cpuset[i]);
  }

  for (int i=0; i<workers; i++) {
    if (pthread_join(work[i].tid, NULL) != 0) {
      printf("failed to wait thread %d \n", i);
    }
  }

  // for (int i=0; i<test_size; i++) {
  //   printf("%d ", results[i]);
  // }
  gettimeofday(&t2, NULL);
  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("done waiting for jobs: execution time: %0.5gms\n", elapsedTime);
}

// ============== experiment ==================

void* worker_func(void* arg) {
  struct work_load* wl = (struct work_load*)arg;

  // printf("worker %lu: range[%d, %d] \n", wl->start, wl->end);
  for (int i=wl->start; i<wl->end; i++) {
    jobs[i].func(jobs[i].aux);
  }
  
  return NULL;
} 

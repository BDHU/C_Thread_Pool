#include<stdlib.h>
#include<stdio.h>
#include <sys/sysinfo.h>
#include "thread.h"

// will probably change to list when the thread number becomes dynamic
Thread* thread_info;

void* worker_func(void* t);

void thread_pool_init(int workers) {
  // set workers to number of processors if not given a defined value
  workers = workers == 0 ? get_nprocs() : workers;
  printf("Initializing thread pool with %d threads \n", workers);  
  
  // initialize threads
  Thread* thread_info = malloc(workers * sizeof(Thread)); 
  if (!thread_info) {
    printf("Failed to initialize thread pool. Exiting now.");
    exit(1);
  }

  int e;
  // set up info struct for each thread
  for (int i=0; i<workers; i++) {
    if ((e=pthread_spin_init(&thread_info[i].lock, PTHREAD_PROCESS_PRIVATE)) != 0) {
      printf("failed to initialize the spinlock for thread %d, error code %d \n", i, e);
      continue; // skip this thread, oops.
    } 

    if (pthread_create(&thread_info[i].tid, NULL, worker_func, &thread_info[i]) != 0) {
      printf("failed to create thread %d \n", i);
    }
  }
}

void* worker_func(void* t) {
  Thread* info = (Thread*) t;
  printf("thread %u is up and running \n", info->tid);

  return NULL;
} 


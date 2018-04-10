#include <stdlib.h>
#include <stdio.h>
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
  int i = 0;
  for (; i<workers; i++) {
      if ((e=pthread_spin_init(&thread_info[i].lock, PTHREAD_PROCESS_PRIVATE)) != 0) {
          printf("failed to initialize the spinlock for thread %d, error code %d \n", i, e);
          continue; // skip this thread, oops.
    } 

      if (pthread_create(&thread_info[i].tid, NULL, worker_func, &thread_info[i]) != 0) {
          printf("failed to create thread %d \n", i);
      }
  }
}

Task* task_init(void *func, void* aux) {
    assert(func != NULL && aux != NULL);
    Task *new_task = (Task *) malloc(sizeof(Task));
    if (!new_task) {
        printf("Task initialization failed\n");
        return NULL;
    }
    // initialize task's attributes
    new_task->func = func;
    new_task->aux = aux;
    // These two variable will be updated when they are added to a thread's queue
    new_task->prev = NULL;
    new_task->next = NULL;
    return new_task;
}

void task_add(Task* task, Thread *thread) {
    // TODO might need lock here
    // also the queue might be empty
    assert(task != NULL && thread != NULL);
    pthread_spin_lock(thread->lock);
    task->prev = thread->last_task;
    thread->last_task->next = task;
    thread->last_task = task;
    total_tasks ++:
    pthread_spin_unlock(thread->lock);
}

// TODO so far we only have a single queue on each thread to keep track
// of all tasks assigned to it, but later we might have multiple queues
// needed to track task at different states such as blocked etc.
void task_remove(Thread *thread) {
    // currently we are
    assert(thread != NULL);
//#if defined(SPIN_LOCK)
    pthread_spin_lock(thread->lock);
    if (thread->last_task == NULL) {
        pthread_spin_unlock(thread->lock);
        return;
    }
    
    // The task we remove can be the only one left in the queue
    if (thread->task_queue == thread->last_task) {
        
    }
    pthread_spin_unlock(thread->lock);
//#endif
}

bool task_free(Task *task) {
    // need to clean up the mess in thread
    free((void *) task);
}

void* worker_func(void* t) {
  Thread* info = (Thread*) t;
  printf("thread %u is up and running \n", info->tid);
  return NULL;
}




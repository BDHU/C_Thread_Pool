#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include "thread.h"

// will probably change to list when the thread number becomes dynamic
Thread* thread_info;
int workers;
int mutex_flag;

void* worker_func(void* t);

void thread_pool_init(int w, int use_mutex) {
  mutex_flag = use_mutex;

  // set workers to number of processors if not given a defined value
  workers = w == 0 ? get_nprocs() : w;
  printf("Initializing thread pool with %d threads \n", workers);  
  
  // initialize threads
  thread_info = calloc(workers, sizeof(Thread)); 
  if (!thread_info) {
      printf("Failed to initialize thread pool. Exiting now.");
      exit(1);
  }

  int e;
  // set up info struct for each thread
  for (int i=0; i<workers; i++) {
    // initialize both just in case
    if ((e=pthread_mutex_init(&thread_info[i].mutex, NULL)) != 0) {
      printf("failed to initialize the mutex for thread %d, error code %d\n", i, e);
      continue;
    }

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
    assert(task != NULL && thread != NULL);
    lock(thread);
    // if the task queue is initially empty 
    if (thread->task_queue == NULL) {
        thread->task_queue = task;
        thread->last_task = task;
        unlock(thread);
        return;
    }
    task->prev = thread->last_task;
    thread->last_task->next = task;
    thread->last_task = task;
    total_tasks ++:
    unlock(thread);
}

// TODO so far we only have a single queue on each thread to keep track
// of all tasks assigned to it, but later we might have multiple queues
// needed to track task at different states such as blocked etc.
void task_remove(Thread *thread) {
    // currently we are
    assert(thread != NULL);
    lock(thread);
    if (thread->last_task == NULL) {
        unlock(thread);
        return;
    }
    
    // The task we remove can be the only one left in the queue
    if (thread->task_queue == thread->last_task) {
        Task *task_to_remove = thread->last_task;
        thread->last = NULL;
        thread->task_queue = NULL;
        task_free(task_to_remove);
        unlock(thread);
        return;
    }

    // normal situation
    Task *task_to_remove = thread->last_task;
    thread->last_task = thread->last_task->prev;
    thread->last_task->next = NULL;
    task_free(task_to_remove);
    unlock(thread);
}

bool task_free(Task *task) {
    // need to clean up the mess in thread
    assert(task != NULL);
    free((void *) task);
}

void lock(Thread* t) {
  int e = 0;
  if (mutex_flag) 
    e = pthread_mutex_lock(&t->mutex);
  else
    e = pthread_spin_lock(&t->lock);

  if (e != 0) {
    printf("failed to grab the lock, error code %d \n", e);
    exit(1);
  }
}

void unlock(Thread* t) {
  int e = 0;
  if (mutex_flag) 
    e = pthread_mutex_unlock(&t->mutex);
  else
    e = pthread_spin_unlock(&t->lock);

  if (e != 0) {
    printf("failed to release the lock, error code %d \n", e);
    exit(1);
  }
}

void* worker_func(void* t) {
  Thread* info = (Thread*) t;
  printf("thread %lu is up and running \n", info->tid);

  Task* task = NULL;
  // grabbing a task from the front of the queue
  // assume task is present
  while(1) {
    // wait for jobs to come in. busy waiting for now
    // TODO: think about blocking/parking policy
    while (info->task_queue == NULL);
    lock(info);
    // state might have changed already
    task = info->task_queue;
    info->task_queue = task->next;
    unlock(info);

    // task may or may not exist
    if (task) {
      task->func(task->aux);
    }
  }

  return NULL;
}


void thread_pool_wait() {
  // wait for threads to finish by checking their task_queues
  for (int i=0; i<workers; i++) {    
    while(thread_info[i].task_queue != NULL);
  }
}

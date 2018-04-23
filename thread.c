#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include "thread.h"

#define CONT_TASK 1

/* will probably change to list when the thread number becomes dynamic */
Thread* thread_info;
_Atomic int task_taker;	/* When new tasks add created this thread is used as the job taker */
int workers;
int mutex_flag;
  
int total_tasks;   

void* worker_func(void* t);
void lock(Thread* t);
void unlock(Thread* t);

Task* task_init(task_func *func, void* aux);
void task_add(Task* task);
Task *grab_task(int num_tasks, Task_Queue* q);

/* ======================== Thread pool ======================== */

void thread_pool_init(int w, int use_mutex) {
  static bool initialized = false;
  mutex_flag = use_mutex;
  // total_tasks = 0;

  /* avoid reinitialization of the thread pool */
  if (initialized) 
    return;

  /* set workers to number of processors if not given a defined value */
  workers = w == 0 ? get_nprocs() : w;
  printf("Initializing thread pool with %d threads \n", workers);  

  /* initialize threads info */
  thread_info = calloc(workers, sizeof(Thread)); 
  if (!thread_info) {
    printf("Failed to initialize thread pool. Exiting now.");
    exit(1);
  }

  int e;

  for (int i=0; i<workers; i++) {
  	/* initialize each thread's semaphore */
    if ((e=sem_init(&thread_info[i].sema, PTHREAD_PROCESS_PRIVATE, 0)) != 0) {
      printf("failed to initialize the semaphore for thread %d, error code %d\n", i, e);
      i--;
      continue;
    }

    /* initialize each thread's mutex */
    if ((e=pthread_mutex_init(&thread_info[i].mutex, NULL)) != 0) {
      printf("failed to initialize the mutex for thread %d, error code %d\n", i, e);
      i--;
      continue;
    }

    /* initialize each thread's tasks queue */
    Task_Queue *ptr = NULL;
    if ((ptr=malloc(sizeof(Task_Queue))) == NULL)
      continue;
    else {
      thread_info[i].queue = ptr;
      thread_info[i].queue->num_tasks = 0;
      thread_info[i].queue->start = NULL;
      thread_info[i].queue->end = NULL;
	}

    /* initialzie each thread's spinlock */
    if ((e=pthread_spin_init(&thread_info[i].lock, PTHREAD_PROCESS_PRIVATE)) != 0) {
      printf("failed to initialize the spinlock for thread %d, error code %d \n", i, e);
      i--;
      continue;
    } 

    /* start executing each thread */
    if (pthread_create(&thread_info[i].tid, NULL, worker_func, &thread_info[i]) != 0) {
      printf("failed to create thread %d \n", i);
      i--;
    }
  }
  task_taker = 0;	/* The first thread is the one begin to be fed in tasks */
  initialized = true;
}

/* Wait for threads to finish by checking their task_queues */
void thread_pool_wait() {
  while(total_tasks != 0);

  // Note: this only works because there can be on thread callling 
  // thread_pool_wait and thread_pool_add. If we want to allow
  // multiple threads to update, we should add a lock to prevent
  // one from happening when waiting is happening.
  for (int i=0; i<workers; i++) 
    while(thread_info[i].queue->num_tasks != 0);
}

// TODO: change so that the main thread will determine which thread to
// add the function
bool thread_pool_add(task_func *func, void* aux) {
  Task* t = task_init(func, aux);
  if (t == NULL) {
    return false;
  }
  task_add(t);
  return true;
}

/* ======================== Task ======================== */

/* Initialize the task executing user-defined function */
Task* task_init(task_func *func, void* aux) {
  if (func == NULL)
    return NULL;

  Task *new_task = (Task *) malloc(sizeof(Task));
  if (!new_task) {
    printf("Task initialization failed\n");
    return NULL;
  }
  
  /* initialize task's attributes */
  new_task->func = func;
  new_task->aux = aux;
  
  /* These two variable will be updated when added to a thread's queue */
  new_task->prev = NULL;
  new_task->next = NULL;
  return new_task;
}

// task add will only be executed by one thread
/* should be used as an internal function */
void task_add(Task* task) {
  if (task == NULL) {
    printf("Warning: you have either a NULL task \n");
    return;
  }
  
  lock(&thread_info[task_taker]);
  // total_tasks++;  // TODO change to private

  Thread *t = &thread_info[task_taker];
  Task_Queue *q = t->queue;

  if (q->num_tasks <= 0) {
  	q->start = task;
  	q->end = task;
  	q->num_tasks = 1;
  	return;
  }

  // Task *temp = q->end;
  task->prev = q->end;
  q->end->next = task;
  q->end = task;
  task->next = NULL;
  (q->num_tasks)++;


  // if (avail_tasks == NULL) {
  //   avail_tasks = task;
  // } else {
  //   task->prev = last_task;
  //   last_task->next = task;
  // }

  // last_task = task;
  unlock(&thread_info[task_taker]);
  
  if (++task_taker >= workers)
  	task_taker = 0;

  // if ((e=sem_post(&info->sema)) != 0) 
  //   printf("Failed to add task, error %d, will keep trying \n", errno);
}

// can specify how many tasks to grab, will do best effort
Task *grab_task(int num_tasks, Task_Queue* q) {
  if (q->num_tasks <= 0)
    return NULL;
  
  // TODO might want to nmove lock to here

  // Task_Queue *t = NULL;
  // lock(NULL);
  // if ((t = malloc(sizeof(Task_Queue))) == NULL)
    // goto done;

  Task *grabbed = q->start;
  if (q->start == q->end) {
  	q->start == NULL;
  	q->end = NULL;
  	goto done;
  }

  q->start = q->start->next;
  q->start->prev = NULL;


  // t->start = avail_tasks;
  // t->end = avail_tasks;
  
  // if (num_tasks < total_tasks) {
  //   // take off the first x number of tasks, set avail_tasks to the next entry.
  //   for (int i=1; i<num_tasks; i++) 
  //     t->end = t->end->next;
  //   avail_tasks = t->end->next;
  //   t->end->next = NULL;  
  //   t->num_tasks = num_tasks;
  // } else {
  //   t->end = last_task;
  //   last_task = NULL;
  //   avail_tasks = NULL;
  //   t->num_tasks = total_tasks; 
  // }
  // *ret = t;
  // // __sync_synchronize();  // barrier is necessary here bc w eare busy waiting on total task to be 0
  // total_tasks -= t->num_tasks;

done:
  q->num_tasks--;
  return grabbed;
  // unlock(NULL);
}

/*
 * This function is executed by each thread
 */
void* worker_func(void* t) {
  Thread* thread = (Thread*) t;

  /* grabbing a task from the front of the queue
     assume task is present */
  while(1) {
    // TODO protection later when multiple threads are executing it
    while(thread->queue->num_tasks == 0);

    // NOTE: need to consider the interaction of sem_wait and 
    // future work stealing algorithm
    // if ((e=sem_wait(&info->sema)) != 0) {
    //   printf("Failed to wait for task, error %d, will keep trying \n", errno);
    //   continue;
    // }
    lock(thread);
    Task *task = grab_task(CONT_TASK, thread->queue);
    unlock(thread);

    if (task) {
    // if (thread->queue) {
      // printf("Thread %d grabbed tasks of size %d \n", thread->tid, tq->num_tasks);
      // private queue. TODO: work stealing will change the story   
      // Task* task = thread->queue->start;
      // while(task != NULL) {
      //   Task* next = task->next;
        task->func(task->aux);
        free(task);
        // task = next;
        // thread->queue->num_tasks--;
      // }
    }
    // thread->queue = NULL; // rethink if storing in thread is even necessary
  }
}

void lock(Thread* t) {
  int e;

  if (mutex_flag) 
    e = pthread_mutex_lock(&t->mutex);
  else
    e = pthread_spin_lock(&t->lock);

done:
  if (e != 0) {
    printf("failed to grab the lock, error code %d \n", e);
    exit(1);
  }
}

void unlock(Thread* t) {
  int e;

  if (mutex_flag) 
    e = pthread_mutex_unlock(&t->mutex);
  else
    e = pthread_spin_unlock(&t->lock);

done:
  if (e != 0) {
    printf("failed to release the lock, error code %d \n", e);
    exit(1);
  }
}

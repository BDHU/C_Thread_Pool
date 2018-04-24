#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include "thread.h"

#define CONT_TASK 1
sem_t task_sema;

/* will probably change to list when the thread number becomes dynamic */
Thread* thread_info;
Task* avail_tasks; // global queue for tasks
Task* last_task;
int workers;
int mutex_flag;

/* guard access to task_queue */
pthread_spinlock_t tasks_lock;     
int total_tasks;   

void* worker_func(void* t);
void lock(Thread* t);
void unlock(Thread* t);

Task* task_init(task_func *func, void* aux);
void task_add(Task* task);
void grab_task(int num_tasks, Task_Queue** ret);

/* ======================== Thread pool ======================== */

void thread_pool_init(int w, int use_mutex) {
  static bool initialized = false;
  mutex_flag = use_mutex;
  avail_tasks = NULL;
  last_task = NULL;
  total_tasks = 0;

  // avoid reinitialization of the thread pool
  if (initialized) 
    return;

  // initialize global semaphore  
  if (sem_init(&task_sema, PTHREAD_PROCESS_PRIVATE, 0) != 0) {
      printf("failed to initialize the semaphore for task \n");
      exit(1);
  }  

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
  if ((e=pthread_spin_init(&tasks_lock, PTHREAD_PROCESS_PRIVATE)) != 0) {
    printf("failed to initialize the spinlock error code %d \n", e);
    exit(1);    
  }

  for (int i=0; i<workers; i++) {
    if ((e=sem_init(&thread_info[i].sema, PTHREAD_PROCESS_PRIVATE, 0)) != 0) {
      printf("failed to initialize the semaphore for thread %d, error code %d\n", i, e);
      i--;
      continue;
    }

    if ((e=pthread_mutex_init(&thread_info[i].mutex, NULL)) != 0) {
      printf("failed to initialize the mutex for thread %d, error code %d\n", i, e);
      i--;
      continue;
    }

    if ((e=pthread_spin_init(&thread_info[i].lock, PTHREAD_PROCESS_PRIVATE)) != 0) {
      printf("failed to initialize the spinlock for thread %d, error code %d \n", i, e);
      i--;
      continue;
    } 

    if (pthread_create(&thread_info[i].tid, NULL, worker_func, &thread_info[i]) != 0) {
      printf("failed to create thread %d \n", i);
      i--;
    }
  }
  initialized = true;
}

/* Wait for threads to finish by checking their task_queues */
void thread_pool_wait() {
  // may need to use a barrier 
  while(total_tasks != 0);

  // Note: this only works because there can be on thread callling 
  // thread_pool_wait and thread_pool_add. If we want to allow
  // multiple threads to update, we should add a lock to prevent
  // one from happening when waiting is happening.
  for (int i=0; i<workers; i++) 
    while(thread_info[i].queue != NULL);
}

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
  // int e;

  if (task == NULL) {
    printf("Warning: you have either a NULL task \n");
    return;
  }
  
  lock(NULL);
  total_tasks++;

  if (avail_tasks == NULL) {
    avail_tasks = task;
  } else {
    task->prev = last_task;
    last_task->next = task;
  }

  last_task = task;
  unlock(NULL);
  // notify
  sem_post(&task_sema);
  // if ((e=sem_post(&info->sema)) != 0) 
  //   printf("Failed to add task, error %d, will keep trying \n", errno);
}

// can specify how many tasks to grab, will do best effort
void grab_task(int num_tasks, Task_Queue** ret) {
  if (ret == NULL)
    return;
  *ret = NULL;
  
  sem_wait(&task_sema);
  Task_Queue *t = NULL;
  lock(NULL);
  if (total_tasks <= 0)
    goto done;
  if ((t = malloc(sizeof(Task_Queue))) == NULL)
    goto done;

  t->start = avail_tasks;
  t->end = avail_tasks;
  
  if (num_tasks < total_tasks) {
    // take off the first x number of tasks, set avail_tasks to the next entry.
    for (int i=1; i<num_tasks; i++) 
      t->end = t->end->next;
    avail_tasks = t->end->next;
    t->end->next = NULL;  
    t->num_tasks = num_tasks;
  } else {
    t->end = last_task;
    last_task = NULL;
    avail_tasks = NULL;
    t->num_tasks = total_tasks; 
  }
  *ret = t;
  __sync_synchronize();  // barrier is necessary here bc w eare busy waiting on total task to be 0
  total_tasks -= t->num_tasks;

done:
  unlock(NULL);
}

void* worker_func(void* t) {
  // int e;
  Thread* thread = (Thread*) t;

  /* grabbing a task from the front of the queue
     assume task is present */
  while(1) {
    // back to busy waiting for now
    // while(avail_tasks == NULL);

    // NOTE: need to consider the interaction of sem_wait and 
    // future work stealing algorithm
    // if ((e=sem_wait(&info->sema)) != 0) {
    //   printf("Failed to wait for task, error %d, will keep trying \n", errno);
    //   continue;
    // }
    grab_task(CONT_TASK, &thread->queue);
    if (thread->queue) {
      // printf("Thread %d grabbed tasks of size %d \n", thread->tid, tq->num_tasks);
      // private queue. TODO: work stealing will change the story   
      Task* task = thread->queue->start;
      while(task != NULL) {
        Task* next = task->next;
        task->func(task->aux);
        free(task);
        task = next;
        thread->queue->num_tasks--;
      }
    }
    thread->queue = NULL; // rethink if storing in thread is even necessary
  }
}

void lock(Thread* t) {
  int e;
  if (t == NULL) {
    e = pthread_spin_lock(&tasks_lock);
    goto done;
  }

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
  if (t == NULL) {
    e = pthread_spin_unlock(&tasks_lock);
    goto done;
  }

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

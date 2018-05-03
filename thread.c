#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <sys/types.h>
#include "thread.h"

#define CONT_TASK 1

/* will probably change to list when the thread number becomes dynamic */
Thread* thread_info;
Task* avail_tasks; // global queue for tasks
Task* last_task;
int workers;
int mutex_flag;
int curr_worker;

/* guard access to task_queue */
pthread_spinlock_t tasks_lock;     

void* worker_func(void* t);
bool trylock(Thread *t);
void lock(Thread* t);
void unlock(Thread* t);

Task* task_init(task_func *func, void* aux);
void assign_task(Task* task);


bool all_done = false;

/* ======================== Thread pool ======================== */

void thread_pool_init(int w, int use_mutex) {
  static bool initialized = false;
  mutex_flag = use_mutex;
  avail_tasks = NULL;
  last_task = NULL;
  curr_worker = 0;

  /* avoid reinitialization of the thread pool */
  if (initialized) 
    return;

  /* set workers to number of processors if not given a defined value */
  //workers = w == 0 ? get_nprocs() : w;
  workers = 8;
  printf("Initializing thread pool with %d threads, mutex %d \n", workers, use_mutex);  
  
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
    if ((e=sem_init(&thread_info[i].task_sema, PTHREAD_PROCESS_PRIVATE, 0)) != 0) {
      printf("failed to initialize the semaphore for thread %d, error code %d\n", i, e);
      i--;
      continue;
    }

    sem_init(&thread_info[i].wait_sema, PTHREAD_PROCESS_PRIVATE, 0);
    
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
  all_done = true;
  initialized = true;
}

/* Wait for threads to finish by checking their task_queues */
void thread_pool_wait() {
  // Note: this only works because there can be on thread callling 
  // thread_pool_wait and thread_pool_add. If we want to allow
  // multiple threads to update, we should add a lock to prevent
  // one from happening when waiting is happening.
  // waiting is not proper
  for (int i=0; i<workers; i++) {
     thread_info[i].count = 1;
     // why is taks sema needed here
     sem_post(&thread_info[i].task_sema);
     sem_wait(&thread_info[i].wait_sema);
     thread_info[i].count = 0;
  }
}

bool thread_pool_add(task_func *func, void* aux) {
  Task* t = task_init(func, aux);
  if (t == NULL) {
    return false;
  }
  assign_task(t);
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
// should be used as an internal function 
void assign_task(Task* task) {
  static int x =0;
  int e;
  if (task == NULL) {
    printf("Warning: you have either a NULL task \n");
    return;
  }
 
  Thread* t = &thread_info[curr_worker];
  lock(t);
  if (t->queue.num_tasks == 0) {
    // if queue is empty
    t->queue.start = t->queue.end = task;
  } else {
    Task *tmp = t->queue.end;
    t->queue.end->next = task;
    t->queue.end = task;
    t->queue.end->prev = tmp;
  }
  t->queue.num_tasks++;
  unlock(t);

  if ((e=sem_post(&t->task_sema)) != 0) 
    printf("Failed to add task, error %d, will keep trying \n", errno);

  x++;
//  if ((x%5) == 0)
   curr_worker = ((curr_worker+1) % workers);
}

bool steal_work(Thread *t, unsigned int *seed) {
  /* randomly choose a thread to steal work form  */
  //unsigned int seed;
  int index = rand_r(seed);
  index %= (workers);
  Thread *victim = thread_info+index;
  if (!all_done || victim == t) {
    return false;
  }
  if (!trylock(victim)) {
    return false;
  }
  if (victim->queue.num_tasks < 1 || victim->count < 1) {
    unlock(victim);
    return false;
  }
  /* remove the task from other  */
  Task *task = NULL;
  task = victim->queue.start;


  (victim->queue.num_tasks) --;
  if (victim->queue.num_tasks == 0) {
    victim->queue.start = NULL;
    victim->queue.end = NULL;
  } else {
    victim->queue.start = victim->queue.start->next; 
    victim->queue.start->prev = NULL;
  }
  unlock(victim);
  if (task) {
    task->func(task->aux);
    free(task);
  }

  return true;

}

void* worker_func(void* t) {
  int e;
  Thread* thread = (Thread*) t;
  printf("thread %li\n", thread->tid);
  unsigned int seed;
  /* grabbing a task from the front of the queue
     assume task is present */
  while(1) {
    // NOTE: need to consider the interaction of sem_wait and 
    // future work stealing algorithm

    // ummm work stealing is gonna be weird lol
    // but whatever, it will be fine.

    /* try to get a task */
    //if ((e=sem_trywait(&thread->task_sema)) != 0) {
    //  printf("Failed to wait for task, error %d, will keep trying \n", errno);
    //  continue;
    //}
  

    // if we failed to decrement the semaphore,
    // that is, we failed to gain the resource to execuet
    // the task
    // if no tasks exists
int times = 0;
    if (sem_trywait(&thread->task_sema) != 0) {
      int i = 0;
      while (true) {
      //for (; i < workers/ 2; i++) {
      bool succeed = steal_work(thread, &seed);
        if (succeed) {
          // use try lock if possible
        } else {
         // break;
         times ++;
        }
        if (times == workers)
          break;
      }
      sem_wait(&thread->task_sema);
    } 

    Task* task = NULL;
    lock(thread);
    if (thread->queue.num_tasks == 0 && thread->count > 0) 
    {
      // wait for global signal send by the main thread
      //if (global_stop == 1) { // check if atomic varible here works
        sem_post(&thread->wait_sema); /* indicates finishing execution */
        printf("finished\n\n");
        break;
      //}
    } else {
      task = thread->queue.start;
      thread->queue.num_tasks--;
      thread->queue.start = task->next;
      task->prev = NULL;
  /*  if (thread->queue.num_tasks == 0) {
      thread->count++;
      thread->queue.end = NULL;
    } else {
      if (thread->queue.num_tasks > thread->max) 
      	thread->max = thread->queue.num_tasks;
    }*/


      // maybe try random stealing here
    }
    unlock(thread);
    
    if (task) {
      task->func(task->aux);
      free(task);
    }
  }
}

bool trylock(Thread *t) {
  int e = -1;
  if (mutex_flag) {
    e = pthread_mutex_trylock(&t->mutex);
    if (e == 0) {
      return true;
    }
  } else {
    e = pthread_spin_trylock(&t->lock);
    if (e == 0) {
      return true;
    }
  }
  return false;
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

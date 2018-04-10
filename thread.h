#include<pthread.h>

typedef struct task {
  void* func;
  void* aux;
  struct task* prev;
  struct task* next;
} Task;

typedef struct thread {
  pthread_t tid;
  pthread_spinlock_t lock; // guard access to task_queue 
  Task* task_queue;
} Thread;

void thread_pool_init(int workers);
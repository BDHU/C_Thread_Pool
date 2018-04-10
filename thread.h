#include<pthread.h>

typedef void task_func (void *aux);

typedef struct task {
  task_func* func;
  void* aux;
  struct task* prev;
  struct task* next;
} Task;

typedef struct thread {
  pthread_t tid;
  pthread_spinlock_t lock; // guard access to task_queue 
  pthread_mutex_t mutex; // guard access to task_queue 
  Task* task_queue;
} Thread;


void thread_pool_init(int workers, int mutex_flag);
void thread_pool_wait();

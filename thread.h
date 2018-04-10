#include<pthread.h>
#include <semaphore.h>

typedef void task_func (void *aux);

typedef struct task {
  void*        aux;
  task_func*   func;
  struct task* prev;
  struct task* next;
} Task;

typedef struct thread {
  pthread_t          tid;
  pthread_spinlock_t lock;        // guard access to task_queue
  pthread_mutex_t    mutex;
  sem_t sema; // use to notify task is there
  int                total_tasks; // number of tasks assigned to this thread TODO atomic maybe?
  Task*              task_queue;
  Task*              last_task;
} Thread;

Task* task_init(task_func *func, void* aux);
// bool add_task_to_thread(pthread_t tid, int num_tasks);

void thread_pool_init(int workers, int mutex_flag);
void thread_pool_wait();

#include <pthread.h>

typedef void task_func (void *aux);

typedef struct task {
  void*        aux;
  ask_func*   func;
  struct task* prev;
  struct task* next;
} Task;

typedef struct thread {
  pthread_t          tid;
  pthread_spinlock_t lock;        // guard access to task_queue
  pthread_mutex_t    mutex;
  int                total_tasks; // number of tasks assigned to this thread TODO atomic maybe?
  Task*              task_queue;
  Task*              last_task;
} Thread;

void thread_pool_init(int workers);
bool task_init();
bool add_task_to_thread(pthread_t tid, int num_tasks);

void thread_pool_init(int workers, int mutex_flag);
void thread_pool_wait();

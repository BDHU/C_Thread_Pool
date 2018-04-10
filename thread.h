#include <pthread.h>

typedef struct task {
  void*        func;
  void*        aux;
  struct task* prev;
  struct task* next;
} Task;

typedef struct thread {
  pthread_t          tid;
  pthread_spinlock_t lock;        // guard access to task_queue
  int                total_tasks; // number of tasks assigned to this thread TODO atomic maybe?
  Task*              task_queue;
  Task*              last_task;
} Thread;

void thread_pool_init(int workers);
bool task_init();
bool add_task_to_thread(pthread_t tid, int num_tasks);

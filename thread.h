#include <pthread.h>
#include <semaphore.h>

typedef void task_func (void *aux);

typedef struct task {
  void*        aux;               /* pointer to arguments of the task */
  task_func*   func;              /* pointer to the function this task will execute */
  struct task* prev;
  struct task* next;
} Task;

typedef struct thread {
  pthread_t          tid;
  pthread_spinlock_t lock;        /* guard access to task_queue */
  pthread_mutex_t    mutex;
  sem_t              sema;        /* use to notify task is there */
  int                total_tasks; /* number of tasks assigned to this thread TODO atomic maybe? */
  Task*              task_queue;
  Task*              last_task;
} Thread;

typedef struct daemon {
  // pointer to the thread pool
  Thread *thread_pool;
  
} Daemon;

/* ======================== user API ======================== */


Task* task_init(task_func *func, void* aux);
void task_add(Task* task, Thread *thread);

void thread_pool_init(int workers, int mutex_flag);
void thread_pool_wait();

/* ======================== Thread pool daemon ======================== */
// TODO ensure daemon is always up
void daemon_init();

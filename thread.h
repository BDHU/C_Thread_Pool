#include <semaphore.h>
#include <stdbool.h>

typedef void* task_func (void *aux);

typedef struct task {
  void*        aux;               /* pointer to arguments of the task */
  task_func*   func;              /* pointer to the function this task will execute */
  struct task* prev;
  struct task* next;
} Task;

// used to hold a segment of tasks
typedef struct task_queue {
  int num_tasks;   
  Task* start;
  Task* end;
} Task_Queue; 

typedef struct thread {
  pthread_t          tid;
  pthread_spinlock_t lock;        /* guard access to task_queue */
  pthread_mutex_t    mutex;
  sem_t              task_sema;        /* use to notify task is there */
  sem_t              wait_sema;        /* use to notify task is there */
  Task_Queue         queue;
  int count;
  int max;
} Thread;

typedef struct temp_thread {
  pthread_t          tid;
  struct temp_thread* next;  
} TempThread;

enum CallType{NonBlocking, Blocking};

/* ======================== user API ======================== */

void thread_pool_init(int workers, int mutex_flag);
bool thread_pool_add(task_func *func, void* aux, enum CallType);
void thread_pool_wait();

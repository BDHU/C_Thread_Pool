// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include <sys/time.h>

int mutex_flag;
const int prime = 4222234741;

void* short_task(void* arg);
void* long_task(void* arg);

int main(int argc, char** argv) {
  int o;
  int workers = 0;
  int test_size = 1000;
  // worker can be predefined or set to default
  struct option opts[3] = {
    {"workers", required_argument, NULL, 'w'},
    {"mutex", no_argument, &mutex_flag, 1},
    { NULL, 0, NULL, 0}
  };

  // parse arguments
  while ((o = getopt_long_only(argc, argv, "w:", opts, NULL)) != -1 ) {
    switch (o) {
      case 0:
      	break;
      case 'w':
        workers = atoi(optarg);
        break;
      default:
        printf("default case, don't recognize anything %d \n", o);
    }
  }

  // worker version
  pthread_t tids[test_size];
  int results[test_size];
  for (int i=0; i<test_size; i++) {
    results[i] = i;
  }
  // does not wake up till later
  struct timeval t1, t2;
  double elapsedTime;
  // start timer
  gettimeofday(&t1, NULL);  
  for (int i=0; i<test_size; i++) {
    if (pthread_create(&tids[i], NULL, short_task, results+i) != 0) {
      printf("failed to create thread %d \n", i);
    }
  }

  for (int i=0; i<test_size; i++) {
    if (pthread_join(tids[i], NULL) != 0) {
      printf("failed to wait thread %d \n", i);
    }
    // int* x = malloc(sizeof(int));
    // thread_pool_add(test1, NULL);  
    // thread_pool_add(test_fib_serires, (void*)(i+3)); 
  }

  gettimeofday(&t2, NULL);
  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("done waiting for jobs: execution time: %0.5gms\n", elapsedTime);
}

// ============== experiment ==================

void* long_task(void* arg) {

} 

// compute sum and then hash it
void* short_task(void* arg) {
  int sum = 0;
  int* p = (int*) arg;
  // compute sum 
  for (int x=1; x<*p; x++) 
    sum += x;

  *p = (sum * (sum-2)) % prime;
}

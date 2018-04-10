// test program
#include<stdlib.h>
#include<stdio.h>
#include <getopt.h>
#include "thread.h"

int mutex_flag;

int main(int argc, char** argv) {
  int o;
  int workers = 0;
  // worker can be predefined or set to default
  struct option opts[3] = {
    {"workers", required_argument, NULL, 'w'},
    {"mutex", no_argument, &mutex_flag, 1},
    { NULL, 0, NULL, 0}
  };

  // parse arguments
  while ((o = getopt_long_only(argc, argv, "w:", opts, NULL)) != -1 ) {
    switch (o) {
      case 'w':
        workers = atoi(optarg);
        break;
      default:
        printf("default case, don't recognize anything %d \n", o);
    }
  }

  thread_pool_init(workers, mutex_flag);
  
  // add jobs
  // need to have a way of knowing when to notify test.c

  // barrier for now?
  // does not wake up till later
  thread_pool_wait();
  printf("done waiting for jobs \n");
  // while(1);
}

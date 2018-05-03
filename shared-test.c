#include <string.h>

void* short_task(void* arg);
void* long_task(void* arg);
void* empty_task(void* arg);
void test1(void* arg);

int rate = 80;
int lnum_limit = 200;
int snum_limit = 800;

const int prime = 4222234741;
const char* data = "A purely peer-to-peer version of electronic cash would allow online\
payments to be sent directly from one party to another without going through a\
financial institution. Digital signatures provide part of the solution, but the main\
benefits are lost if a trusted third party is still required to prevent double-spending.\
We propose a solution to the double-spending problem using a peer-to-peer network.\
The network timestamps transactions by hashing them into an ongoing chain of\
hash-based proof-of-work, forming a record that cannot be changed without redoing\
the proof-of-work. The longest chain not only serves as proof of the sequence of\
events witnessed, but proof that it came from the largest pool of CPU power. As\
long as a majority of CPU power is controlled by nodes that are not cooperating to\
attack the network, they'll generate the longest chain and outpace attackers. The\
network itself requires minimal structure. Messages are broadcast on a best effort\
basis, and nodes can leave and rejoin the network at will, accepting the longest\
proof-of-work chain as proof of what happened while they were gone. \n";

struct out {
  char* dir;
  int arg;
};

void* long_task(void* arg) {
  char buf[30];
  struct out* o = (struct out*) arg;
  snprintf(buf, 30, "%s/tmp-%d", o->dir, o->arg);
  
  FILE *f = fopen(buf, "w+");
  if (!f) {
    printf("Failed to create file \n");
    return NULL;
  }

  size_t size = strlen(data)+1;
  size_t wsize = fwrite(data, 1, size, f);
  if (wsize != size)
    printf("Failed to write %lu bytes, only wrote %lu \n", size, wsize);
  return NULL;
}

// compute sum and then hash it
void* short_task(void* arg) {
  int sum = 0;
  int* p = (int*) arg;
  // compute sum 
  for (int x=1; x<*p; x++) 
    sum += x;

  *p = (sum * (sum-2)) % prime;
  return NULL;
}

void* empty_task(void* arg) {
  return NULL;
}

void test1(void* arg) {
  for(int i=0; i<1000000; i++);
}
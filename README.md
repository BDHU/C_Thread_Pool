Thread Pool Proposal

revision: This looks great. One minor revision request: please submit a revised proposal that explicitly lists what graphs you plan to have in your write up so I can have a concrete understanding of your experiment design(s).

Goals: Implement a thread pool that provides the following features:
*  Reasonable performance compared to Go
*  Ensures progress regardless of malicious or long running tasks
*  Low overhead between worker threads switching compared to pthreads
*  Proper synchronization between worker threads
*  Efficient and automatic load balancing


Timeline:
Milestone 1: Due April 7th
* Study the current state of Go scheduler
* Define abstractions for thread pool
   * Worker thread abstraction
   * Tasks abstraction
   * Data structure associated to each worker thread
   * Define thread pool API


Milestone 2: Due April 14th
* Implement a basic thread pool 
   * Start with M number of worker threads where M = number of processors
* Thread pool daemon to monitor threads state
   * Detect blocking worker threads and create replacement workers
   * Create more workers when current workers have long running tasks
   * Start implementing necessary thread pool API


Milestone 3: Due April 21st
* Dynamic load balancing 
   * Work stealing
   * Test and evaluate the overhead to make sure that the strategies do not degrade performance
   * Improve scheduling algorithm efficiency
* Refine work distribution for locality (tasks created closely in time)
   * Try out different heuristics and evaluate
* Add priority support to user
   * Allow users to assign priority to tasks (think through whether or not the priority should be evaluated per threads or as globally). This will be part of the thread pool API implementation.
   * Should still guarantee some version of bounded waiting. This can be the job assigned to the monitor daemon.


Milestone 4: Due April 28th
* Build communication mechanism to facilitate communication between user application and tasks (or among tasks), the communication is supposed to be lightweight.
* Refine heuristics and apply possible policy. We will decide on the best scheduling algorithm for worker threads
* Test out different data structure used by the thread pool if any resource contention is exposed (frequent access to global queues)
* Run experiments of different work loads to compare with pthreads and goroutines. Analyze results and dig into possible improvements if our thread pool performance does not exceed existing implementation.


Milestone 5: Due May 1st
* Documentation
* Presentation




Infrastructure:
The implementation will heavily rely on the Linux environment since our implementation will heavily rely on pthread. All the tools we need are available in Linux and thus any Linux distro will be enough.


Hardware:
Usual lab machines with intel Xeon processor


Experiments:
Evaluate performance for different workloads:
* How does task with blocking system call affect our performance?
   *  Tests for tasks with different percentage of system calls (90%, 50%, 5%)
* Use multiple synchronization methods(mutex, spinlock, barrier) to test performance differences. (maybe this should be in a separate bullet point? Do you mean using these primitives as blocking system calls?) 
* Record execution time of running a mixture of long and short jobs and compare with pthreads and goroutines.
* Compare throughput with pthreads and goroutines.
* Evaluate if our thread pool can still make progress in the presence of malicious/buggy user tasks(ex. infinite loop)


Success Metrics:
If we outperform at least the pthreads or goroutine with the same test cases since Go has GC function which can be a performance overhead.


Expected Challenges:


* What kind of performance or functionality problems do you anticipate?
   * Ensure worker threads availability under any circumstances(not blocked)?
   * Synchronization between worker threads(accessing each other’s queue)
   * The efficiency of scheduling algorithm might not work as expected because the system scheduler can interfere with it.
   * Monitoring threads state: ensures progress
   * Starvation (for priority)
   * Possibly priority inversion
   * Thread state thrashing (frequent updates)
   * Communication between worker threads on different pthreads


Reference Documents:
Go scheduler code
Scalable Go Scheduler Design Doc
Work stealing paper
Go scheduler overview
Go runtime scheduler analysis

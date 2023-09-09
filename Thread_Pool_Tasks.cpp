/*
    Author: Fazrian Prawiranata
    Data: 09/11/2023
    Notes: A refresh on pthreads for parallelism
*/

#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <queue>

struct work_pack
{
    int             id, num;  // User Query
    pthread_cond_t* wait; // Used to make the thread wait for work
    work_pack(int i_id = 0, int i_num = 1) 
    {
        id = i_id;
        num = i_num;
        wait = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
        pthread_cond_init(wait, NULL);
    }
};

struct task_manager
{
    pthread_mutex_t*      lock;   // Global Lock
    std::queue<work_pack> prog;   // Stores all the working threads
    std::queue<work_pack> avail;  // Stores all the available threads
};

struct thread_pack
{
    int id;
    void* work;
    void* global;
};

void* prime_thread(void* args);

int main(int argc, char** argv)
      // usage: /.Thread_Pool [Opt. Num_Threads]
{
    task_manager thread_pool;
    int num_thread = 10;
    if(argc >= 2)
      num_thread = std::stoi(argv[1]);

      // Spin specified number of threads and have them wait for tasks
    for(int i = 0; i < num_thread; ++i)
    {

    }  
      // Await user input in a loop. If user specifies a number assign a thread
        // If user specifies an action, do the action
      
      // When Stop is specified AND TASK MANAGER IS EMPTY, break out and clean up

    return 0;
}

void* prime_thread(void* args)
{
    thread_pack* arg = (thread_pack*)args;
    work_pack* work = (work_pack*)arg->work;
    task_manager* global = (task_manager*)arg->global;

    while(1)
    {
          //Lock the mutex then wait for work
        pthread_mutex_lock(global->lock);
        pthread_cond_wait(work->wait, global->lock);
          //Once signaled work can begin
          //If signaled num is -1 break and destroy
        if(work->num == -1)
          break;
    }

    pthread_cond_destroy(work->wait);
}
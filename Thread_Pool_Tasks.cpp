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
    int               id, num;  // User Query
    pthread_t         tid;
    pthread_cond_t*   wait;     // Used to make the thread wait for work
    pthread_mutex_t*  lock;     // 
    work_pack(int i_id = 0, int i_num = 1) 
    {
        id = i_id;
        num = i_num;
        wait = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
        pthread_cond_init(wait, NULL);
    }
    ~work_pack() { pthread_cond_destroy(wait); }
};

struct task_manager
{
    pthread_mutex_t*      lock;   // Global Lock
    std::queue<work_pack*> prog;   // Stores all the working threads
    std::queue<work_pack*> avail;  // Stores all the available threads
    task_manager() 
    { 
        lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t)); 
        pthread_mutex_init(lock, NULL);
    }
};

struct thread_pack
{
    pthread_t   tid;
    void*       work;
    void*       global;
    thread_pack(task_manager* i_g, work_pack* i_w)
    {
      work = (void*)i_w;
      global = (void*)i_g;
    }
};

void pause_resume();
void* prime_thread(void* args);
void print_usage();

int main(int argc, char** argv)
      // usage: /.Thread_Pool [Opt. Num_Threads]
{
    task_manager* thread_pool = new task_manager;
    int num_thread = 10;
    if(argc >= 2)
      num_thread = std::stoi(argv[1]);
      // Spin specified number of threads and have them wait for tasks
    for(int i = 0; i < num_thread; ++i)
    {
      work_pack* tmp_w = new work_pack(i);
      thread_pack* tmp = new thread_pack(thread_pool, tmp_w);
      if (pthread_create(&(tmp->tid), NULL, prime_thread, (void*)tmp) != 0)
          perror("pthread_create(Chat_Room)");
      thread_pool->avail.push(tmp_w);
    }  
      // Await user input in a loop. If user specifies a number assign a thread
        // If user specifies an action, do the action
    print_usage();
    std::string buffer;
    bool isPaused = false;
    while(std::cin >> buffer)
    {
      if(buffer.compare("stop") == 0) 
        break;
      else if(buffer.compare("pause") == 0 && isPaused == false) 
      {
        pause_resume(); 
        isPaused = true;
      }
      else if(buffer.compare("resume") == 0 && isPaused == true) 
      {
        pause_resume(); 
        isPaused = false;
      }

      printf("Available Threads: %ld\n", thread_pool->avail.size());
    }
      // When Stop is specified AND TASK MANAGER IS EMPTY, break out and clean up

    return 0;
}

void print_usage()
{
    printf("Usage(case sensitive):\n\t[#] - Input a number to see if it's prime or not\n");
    printf("\tpause - pause all threads\n\tresume - resume all threads(does nothing if not paused)\n");
    printf("\tstop - end the program\n\thelp - see this menue again\n");
}

void pause_resume()
{
    // Just grab the mutex
}

void* prime_thread(void* args)
{
    thread_pack* arg = (thread_pack*)args;
    work_pack* work = (work_pack*)arg->work;
    task_manager* global = (task_manager*)arg->global;
    work->tid = arg->tid;
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
}
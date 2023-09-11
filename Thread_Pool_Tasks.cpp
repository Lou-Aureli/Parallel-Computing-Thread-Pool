/*
    Author: Fazrian Prawiranata (fprawira)
    Data: 09/11/2023
    Notes:  A refresh on pthreads for parallelism. Please use on a unix system (Ubuntu or SSH hydra)
            For example code please use the provided input.txt and do ./Thread_Pool < input.txt
            A simple "make" should compile the thing.
*/

#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <queue>
#include <unistd.h>

struct work_pack
{
    int               id, num;  // User Query
    pthread_t         tid;
    pthread_cond_t*   wait;     // Used to make the thread wait for work
    work_pack(int i_id = 0, int i_num = 1) 
    {
        id = i_id;
        num = i_num;
        wait = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
        pthread_cond_init(wait, NULL);
    }
    ~work_pack() 
    { 
        pthread_cond_destroy(wait);
        free(wait);
    }
};

struct task_manager
{
    pthread_mutex_t*      lock;   // Global Lock
    std::queue<work_pack*> prog;  // Stores all the working threads
    std::queue<work_pack*> avail;  // Stores all the available threads
    task_manager() 
    { 
        lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t)); 
        pthread_mutex_init(lock, NULL);
    }
    ~task_manager()
    {
        pthread_mutex_unlock(lock);
        pthread_mutex_destroy(lock);
        free(lock);
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
    int user_num;
    std::string buffer;
    bool is_Paused = false;
    print_usage();
    while(std::cin >> buffer)
    {
        printf("Available Threads: %ld\n", thread_pool->avail.size());
        if(buffer.compare("stop") == 0)
            break;
        else if(buffer.compare("pause") == 0 && is_Paused == false) 
        {
            pthread_mutex_lock(thread_pool->lock); 
            printf(">Paused\n");
            is_Paused = true;
        }
        else if(buffer.compare("pause") == 0 && is_Paused == true) 
            printf(">Already paused\n");
        else if(buffer.compare("resume") == 0 && is_Paused == true) 
        {
            pthread_mutex_unlock(thread_pool->lock); 
            printf(">Resume\n"); 
            is_Paused = false;
        }
        else if(buffer.compare("resume") == 0 && is_Paused == false) 
            printf(">Must be paused to resume\n"); 
        else if(buffer.compare("help") == 0)
        {
            print_usage();
        }
        else if(isdigit(buffer[0]))
        {
            if(thread_pool->avail.size() == 0)
            {
                printf(">Please wait for a thread to become available.\n");
                continue;
            }
            work_pack* tmp = thread_pool->avail.front();
            user_num = std::stoi(buffer);
            thread_pool->avail.pop();
            tmp->num = user_num;
            pthread_cond_signal(tmp->wait);
        }
        else
            printf(">Invalid order please input a numeric value or 'help' for command list\n");
    }
      // When Stop is specified ALL threads are returned then clean up
    printf(">Stop order recieved, resuming if paused and awaiting all threads return\n");
    if(is_Paused)
        pthread_mutex_unlock(thread_pool->lock); 
    while(thread_pool->avail.size() < 10);
    return 0;
}

void print_usage()
{
    printf("Usage(case sensitive):\n\t[#] - Input a number to see if it's prime or not\n");
    printf("\tpause - pause all threads\n\tresume - resume all threads(does nothing if not paused)\n");
    printf("\tstop - end the program\n\thelp - see this menue again\n");
}

void* prime_thread(void* args)
{
    int sleep_num = 5;
    bool is_Prime = true;
    thread_pack* arg = (thread_pack*)args;
    work_pack* work = (work_pack*)arg->work;
    task_manager* global = (task_manager*)arg->global;
    work->tid = arg->tid;
    
    while(1)
    {
            // Lock the mutex then wait for work
        pthread_mutex_lock(global->lock);
        pthread_cond_wait(work->wait, global->lock);
        pthread_mutex_unlock(global->lock);
    
            // Since the program goes by quick I wanted to give a bit of time
            // So the user could pause and accumulate tasks then resume
        printf("\tWorker Thread %d: I got your order of %d, gonna nap for %ds then work\n", work->id, work->num, sleep_num);
        sleep(sleep_num);
        printf("\tWorker Thread %d: I'm up and working!\n", work->id);
        
            // Once signaled work can begin
        if(abs(work->num) <= 1)
          is_Prime = false;
        for(int i = 2; i < (abs(work->num)/2); ++i)
        {
          if((work->num % i) == 0)
          {
              is_Prime = false;
              break;
          }
        }
        if(is_Prime)
            printf("\tWorker Thread %d: The number %d is prime!\n", work->id, work->num);
        else
            printf("\tWorker Thread %d: The number %d is not prime!\n", work->id, work->num);
          // When done put self back on avail queue
        global->avail.push(work);
    }
}
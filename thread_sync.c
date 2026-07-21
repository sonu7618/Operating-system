#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define INCREMENTS_PER_THREAD 20000

//Shared Variables 
long shared_counter_unsafe = 0;
long shared_counter_safe = 0;

//Mutex for synchronization 
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutexes for deadlock prevention 
pthread_mutex_t lock_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_B = PTHREAD_MUTEX_INITIALIZER;

//Race Condition 
void *unsafe_increment(void *arg)
{
    long id = (long)arg;

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++)
    {
        long temp = shared_counter_unsafe;

        for (volatile int busy = 0; busy < 50; busy++)
        {
        }

        temp++;
        shared_counter_unsafe = temp;
    }

    printf("Thread %ld finished.\n", id);

    return NULL;
}

//Mutex Synchronization
void *safe_increment(void *arg)
{
    long id = (long)arg;

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++)
    {
        pthread_mutex_lock(&counter_mutex);

        shared_counter_safe++;

        pthread_mutex_unlock(&counter_mutex);
    }

    printf("Thread %ld finished.\n", id);

    return NULL;
}

//Deadlock Prevention
void *worker(void *arg)
{
    long id = (long)arg;

    for (int i = 0; i < 3; i++)
    {
        pthread_mutex_lock(&lock_A);
        printf("Thread %ld locked A\n", id);

        usleep(1000);

        pthread_mutex_lock(&lock_B);
        printf("Thread %ld locked B\n", id);

        printf("Thread %ld is working...\n", id);

        pthread_mutex_unlock(&lock_B);
        pthread_mutex_unlock(&lock_A);

        usleep(500);
    }

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];


    printf("\nDemo 1: Race Condition\n");
    printf("Expected Counter = %d\n",
           NUM_THREADS * INCREMENTS_PER_THREAD);

    shared_counter_unsafe = 0;

    for (long i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, unsafe_increment, (void *)i);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("Unsafe Counter = %ld\n", shared_counter_unsafe);
    printf("Lost Updates = %d\n",
           NUM_THREADS * INCREMENTS_PER_THREAD - (int)shared_counter_unsafe);

    printf("\nDemo 2: Mutex Synchronization\n");

    shared_counter_safe = 0;

    for (long i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, safe_increment, (void *)i);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("Safe Counter = %ld\n", shared_counter_safe);

    printf("\nDemo 3: Deadlock Prevention\n");

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, worker, (void *)1);
    pthread_create(&t2, NULL, worker, (void *)2);
    pthread_create(&t3, NULL, worker, (void *)3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("All threads completed successfully.\n");

    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&lock_A);
    pthread_mutex_destroy(&lock_B);

    return 0;
}
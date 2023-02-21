#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_THREADS 26  //changed for online
#define NUM_ITER 10

zem_t parent;
zem_t t[NUM_THREADS];

void* justprint(void* data)
{
    int thread_id = *((int*) data);
    while(1)
    {
        zem_down(&t[thread_id]);
        char a = 'z'-thread_id; //changed for online
        printf("%c\n",a);
        zem_up(&t[(thread_id+1)%NUM_THREADS]);
    }
    if (thread_id == NUM_THREADS-1)
        zem_up(&parent);
    return 0;
}

int main(int argc, char* argv[])
{

    pthread_t mythreads[NUM_THREADS];
    int mythread_id[NUM_THREADS];

    zem_init(&parent, 0);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (i==0)
        {
            zem_init(&t[i], 1);
        }
        else
        {
            zem_init(&t[i], 0);
        }

        mythread_id[i] = i;
        pthread_create(&mythreads[i], NULL, justprint, (void*) &mythread_id[i]);
    }
    zem_down(&parent);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(mythreads[i], NULL);
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size, item_to_consume;
int total_items, max_buf_size, num_workers, num_masters;
int worker_count = 0, producer_count = 0;

pthread_mutex_t mw_lock = PTHREAD_MUTEX_INITIALIZER; // mutex lock for the shared buffer
pthread_cond_t full_cond = PTHREAD_COND_INITIALIZER; // workers wait for masters to fill buffer if buffer is empty
pthread_cond_t empty_cond = PTHREAD_COND_INITIALIZER; // masters wait for workers to empty buffer if buffer is full

int* buffer;

void print_produced(int num, int master) {
    printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {
    printf("Consumed %d by worker %d\n", num, worker);

}

//produce items and place in buffer
//modify code below to synchronize correctly
void* generate_requests_loop(void* arg) {
    int thread_id = *((int*) arg);
    while (item_to_produce < total_items) {
        pthread_mutex_lock(&mw_lock);
        while (curr_buf_size == max_buf_size && item_to_produce < total_items) {
            pthread_cond_wait(&empty_cond, &mw_lock);
        }
        if (item_to_produce == total_items) {
            pthread_mutex_unlock(&mw_lock);
            break;
        }
        buffer[curr_buf_size++] = item_to_produce;
        print_produced(item_to_produce, thread_id);
        item_to_produce++;
        pthread_cond_broadcast(&full_cond);
        pthread_mutex_unlock(&mw_lock);
    }
    return NULL;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item
void* worker_working_loop(void* arg) {
    int thread_id = *((int*) arg);
    while (item_to_consume < total_items) {
        pthread_mutex_lock(&mw_lock);
        while (curr_buf_size == 0 && item_to_consume < total_items) {
            pthread_cond_wait(&full_cond, &mw_lock);
        }
        if (item_to_consume == total_items) {
            pthread_mutex_unlock(&mw_lock);
            break;
        }
        int consumed = buffer[--curr_buf_size];
        print_consumed(consumed, thread_id);
        item_to_consume++;
        pthread_cond_broadcast(&empty_cond);
        pthread_mutex_unlock(&mw_lock);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Invalid usage: ./main total_items max_buf_size num_workers num_masters\n");
        return 1;
    }

    int* master_thread_id;
    pthread_t* master_thread;
    item_to_produce = 0;
    curr_buf_size = 0;

    int* worker_thread_id;
    pthread_t* worker_thread;
    item_to_consume = 0;

    int i;

    if (argc < 5) {
        printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
        exit(1);
    }
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
    num_workers = atoi(argv[3]);
    num_masters = atoi(argv[4]);

    buffer = (int*) malloc(sizeof(int) * max_buf_size);

    //create master producer threads
    master_thread_id = (int*) malloc(sizeof(int) * num_masters);
    master_thread = (pthread_t*) malloc(sizeof(pthread_t) * num_masters);
    for (i = 0; i < num_masters; i++)
        master_thread_id[i] = i;

    for (i = 0; i < num_masters; i++)
        pthread_create(&master_thread[i], NULL, generate_requests_loop, (void*) &master_thread_id[i]);

    //create worker consumer threads
    worker_thread_id = (int*) malloc(sizeof(int) * num_workers);
    worker_thread = (pthread_t*) malloc(sizeof(pthread_t) * num_workers);

    for (i = 0; i < num_workers; i++)
        worker_thread_id[i] = i;

    for (i = 0; i < num_workers; i++)
        pthread_create(&worker_thread[i], NULL, worker_working_loop, (void*) &worker_thread_id[i]);

    //wait for all threads to complete
    for (i = 0; i < num_masters; i++)
    {
        pthread_join(master_thread[i], NULL);
        printf("master %d joined\n", i);
    }

    for (i = 0; i < num_workers; i++)
    {
        pthread_join(worker_thread[i], NULL);
        printf("worker %d joined\n", i);
    }

    /*----Deallocating Buffers---------------------*/
    free(buffer);
    free(master_thread_id);
    free(master_thread);
    free(worker_thread_id);
    free(worker_thread);

    return 0;
}

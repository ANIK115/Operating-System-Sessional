#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int N = 0;
int curr = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* func(void* arg) {
    int thread_id = *((int *) arg);
    // printf("Running thread %d\n", thread_id);

    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&lock);
        while (curr != thread_id) {
            pthread_cond_wait(&cond, &lock);
        }
        printf("%d%c", thread_id, " \n"[thread_id == N-1]);
        curr = (curr + 1) % N;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Invalid usage: ./main N");
        exit(1);
    }
    N = atoi(argv[1]);
    printf("N = %d\n", N);

    int* thread_ids = (int*) malloc(sizeof(int) * N);
    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * N);
    for (int i = 0; i < N; ++i) {
        thread_ids[i] = i;
        // printf("Starting thread %d\n", thread_ids[i]);
        pthread_create(&threads[i], NULL, func, (void *) &thread_ids[i]);
    }
    for (int i = 0; i < N; ++i) {
        pthread_join(threads[i], NULL);
        // printf("Thread %d has ended\n", i);
    }

    free(threads);
    return 0;
}

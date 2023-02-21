#include <stdio.h>
#include <pthread.h>

unsigned int count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void* func(void* arg) {
    for (int i = 0; i < 10000; ++i) {
        pthread_mutex_lock(&count_mutex);
        ++count;
        pthread_mutex_unlock(&count_mutex);
    }
}

int main(int argc, char* argv[]) {
    pthread_t threads[10];
    for (int i = 0; i < 10; ++i) {
        printf("Starting thread %d\n", i);
        pthread_create(&threads[i], NULL, func, NULL);
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(threads[i], NULL);
        printf("Thread %d has ended\n", i);
    }
    printf("Final value of count = %d", count);
    return 0;
}

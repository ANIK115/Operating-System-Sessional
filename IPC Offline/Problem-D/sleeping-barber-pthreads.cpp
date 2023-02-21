/**
 * The Sleeping Barber Problem
 * A barbershop consists of a waiting room with n chairs, and the
 * barber room containing the barber chair. If there are no customers
 * to be served, the barber goes to sleep. If a customer enters the
 * barbershop and all chairs are occupied, then the customer leaves
 * the shop. If the barber is busy, but chairs are available, then the
 * customer sits in one of the free chairs. If the barber is asleep, the
 * customer wakes up the barber. Write a program to coordinate the
 * barber and the customers.
*/

#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

int no_of_barbers;
int no_of_chairs;
int no_of_customers;

int seen_customer_count = 0;
int customer_count = 0;
int working_barber_cnt = 0;
int barber_done = 0;
int customer_done = 0;

pthread_mutex_t customer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t customer_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t barber_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barber_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t barber_done_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barber_done_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t customer_done_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t customer_done_cond = PTHREAD_COND_INITIALIZER;

void* barber(void* arg) {
    int id = *((int*) arg);
    printf("Barber entered the shop.\n");

    while (seen_customer_count < no_of_customers) {
        // check if there is any customer waiting
        pthread_mutex_lock(&customer_lock);
        if (customer_count == 0) {
            printf("Barber is going to sleep.\n");
            pthread_cond_wait(&customer_cond, &customer_lock);
        }
        pthread_mutex_unlock(&customer_lock);

        printf("Barber started working.\n");
        // notify customer that his turn has come
        pthread_mutex_lock(&barber_lock);
        //this is similar to done = 1
        //customer will have a while loop on this working_barber_cnt variable
        ++working_barber_cnt;       
        pthread_cond_signal(&barber_cond);      //this will wake the customer
        pthread_mutex_unlock(&barber_lock);

        sleep(2);

        // notify customer that barber is done
        pthread_mutex_lock(&barber_done_lock);
        ++barber_done;
        printf("Barber is done.\n");
        pthread_cond_signal(&barber_done_cond);
        pthread_mutex_unlock(&barber_done_lock);

        // wait for customer to be done
        pthread_mutex_lock(&customer_done_lock);
        while (customer_done == 0) {
            pthread_cond_wait(&customer_done_cond, &customer_done_lock);
        }
        --customer_done;
        pthread_mutex_unlock(&customer_done_lock);
    }

    printf("Barber %d is leaving the shop.\n", id);
    return NULL;
}

void* customer(void* arg) {
    int id = *((int*) arg);
    printf("Customer %d entered the shop.\n", id);

    // check if a seat is available and notify barber of customer's arrival
    pthread_mutex_lock(&customer_lock);
    if (customer_count == no_of_chairs) {
        printf("Customer %d cannot find a seat. He is leaving.\n", id);
        ++seen_customer_count;
        pthread_mutex_unlock(&customer_lock);
        return NULL;
    }
    ++customer_count;
    printf("Customer %d is waiting.\n", id);
    pthread_cond_signal(&customer_cond);
    pthread_mutex_unlock(&customer_lock);

    // wait for barber's signal
    //waiting chair a thaka customers ekhane eshe barber er signal er jonne wait korbe
    pthread_mutex_lock(&barber_lock);
    while (working_barber_cnt == 0) {
        pthread_cond_wait(&barber_cond, &barber_lock);
    }
    --working_barber_cnt;
    pthread_mutex_unlock(&barber_lock);

    printf("Customer %d is getting a haircut.\n", id);

    // wait till barber is done
    pthread_mutex_lock(&barber_done_lock);
    while (barber_done == 0) {
        pthread_cond_wait(&barber_done_cond, &barber_done_lock);
    }
    --barber_done;
    pthread_mutex_unlock(&barber_done_lock);

    // update customer count
    pthread_mutex_lock(&customer_lock);
    --customer_count;
    ++seen_customer_count;
    pthread_mutex_unlock(&customer_lock);

    // notify barber that customer is done
    pthread_mutex_lock(&customer_done_lock);
    ++customer_done;
    printf("Customer %d is leaving the shop.\n", id);
    pthread_cond_signal(&customer_done_cond);
    pthread_mutex_unlock(&customer_done_lock);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Invalid usage: ./main no_of_chairs no_of_customers");
        return 1;
    }

    printf("Openned shop.\n");

    no_of_barbers = 1;
    no_of_chairs = atoi(argv[1]);
    no_of_customers = atoi(argv[2]);

    vector<int> barber_thread_ids(no_of_barbers);
    vector<pthread_t> barber_threads(no_of_barbers);
    for (int i = 0; i < no_of_barbers; ++i) {
        barber_thread_ids[i] = i;
        pthread_create(&barber_threads[i], NULL, barber, (void*) &barber_thread_ids[i]);
    }

    vector<int> customer_thread_ids(no_of_customers);
    vector<pthread_t> customer_threads(no_of_customers);
    for (int i = 0; i < no_of_customers; ++i) {
        customer_thread_ids[i] = i;
        pthread_create(&customer_threads[i], NULL, customer, (void*) &customer_thread_ids[i]);
        sleep(rand() % 2);
    }

    for (int i = 0; i < no_of_barbers; ++i) {
        pthread_join(barber_threads[i], NULL);
    }
    for (int i = 0; i < no_of_customers; ++i) {
        pthread_join(customer_threads[i], NULL);
    }

    printf("Shop closed for today.\n");

    return 0;
}

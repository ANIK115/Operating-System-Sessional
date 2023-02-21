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
#include "zemaphore.c"

using namespace std;

int no_of_barbers;
int no_of_chairs;
int no_of_customers;    //total customers from the beginning

int seen_customer_count = 0;
int customer_count = 0;     //currently customers count in the shop

zem_t customer_lock_zem;
zem_t customer_cond_zem;
zem_t barber_lock_zem;
zem_t barber_cond_zem;
zem_t barber_done_lock_zem;
zem_t barber_done_cond_zem;
zem_t customer_done_lock_zem;
zem_t customer_done_cond_zem;

void* barber(void* arg) {
    int id = *((int*) arg);
    printf("Barber entered the shop.\n");

    while (seen_customer_count < no_of_customers) {
        // check if there is any customer waiting
        if (customer_count == 0) {
            printf("Barber is going to sleep.\n");
        }
        zem_down(&customer_cond_zem);

        printf("Barber started working.\n");
        // notify customer that his turn has come
        zem_up(&barber_cond_zem);

        sleep(2);

        // notify customer that barber is done
        zem_up(&barber_done_cond_zem);
        printf("Barber is done.\n");

        // wait for customer to be done
        zem_down(&customer_done_cond_zem);
    }

    printf("Barber %d is leaving the shop.\n", id);
    return NULL;
}

void* customer(void* arg) {
    int id = *((int*) arg);
    printf("Customer %d entered the shop.\n", id);

    // check if a seat is available and notify barber of customer's arrival
    zem_down(&customer_lock_zem);       //acquiring customer lock
    if (customer_count == no_of_chairs) {
        printf("Customer %d cannot find a seat. He is leaving.\n", id);
        ++seen_customer_count;
        zem_up(&customer_lock_zem);
        return NULL;
    }
    ++customer_count;
    printf("Customer %d is waiting.\n", id);
    zem_up(&customer_cond_zem);     //This will signal barber to wake up from sleep and cut hair
    zem_up(&customer_lock_zem);     //Releasing the customer lock acquired above

    // wait for barber's signal
    zem_down(&barber_cond_zem);     //Barber will let this customer know when his turn has come

    printf("Customer %d is getting a haircut.\n", id);

    // wait till barber is done
    zem_down(&barber_done_cond_zem);

    // update customer count
    zem_down(&customer_lock_zem);       //acquiring customer lock
    --customer_count;
    ++seen_customer_count;
    
    zem_up(&customer_lock_zem);     //Releasing customer lock

    printf("Customer %d is leaving the shop.\n", id);
    // notify barber that customer is done
    zem_up(&customer_done_cond_zem);

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

    zem_init(&barber_lock_zem, 1);
    zem_init(&customer_lock_zem, 1);
    zem_init(&barber_done_lock_zem, 1);
    zem_init(&customer_done_lock_zem, 1);

    zem_init(&barber_cond_zem, 0);
    zem_init(&customer_cond_zem, 0);
    zem_init(&barber_done_cond_zem, 0);
    zem_init(&customer_done_cond_zem, 0);

    vector<int> barber_thread_ids(no_of_barbers);
    vector<pthread_t> barber_threads(no_of_barbers);
    for (int i = 0; i < 1; ++i) {
        barber_thread_ids[i] = i;
        pthread_create(&barber_threads[i], NULL, barber, (void*) &barber_thread_ids[i]);
    }

    vector<int> customer_thread_ids(no_of_customers);
    vector<pthread_t> customer_threads(no_of_customers);
    for (int i = 0; i < no_of_customers; ++i) {
        customer_thread_ids[i] = i;
        pthread_create(&customer_threads[i], NULL, customer, (void*) &customer_thread_ids[i]);
        sleep(rand() % 3);
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

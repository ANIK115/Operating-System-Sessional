#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock* rw)
{
    pthread_mutex_init(&rw->r_mutex, NULL);
    pthread_mutex_init(&rw->w_mutex, NULL);
    pthread_mutex_init(&rw->rw_mutex, NULL);
    pthread_cond_init(&rw->w_cond, NULL);
    rw->curr_r_cnt = 0;
    rw->waiting_w_cnt = 0;
}

void ReaderLock(struct read_write_lock* rw)
{
    pthread_mutex_lock(&rw->w_mutex);
    while (rw->waiting_w_cnt > 0) {
        pthread_cond_wait(&rw->w_cond, &rw->w_mutex);
    }
    pthread_mutex_lock(&rw->r_mutex);
    ++rw->curr_r_cnt;
    if (rw->curr_r_cnt == 1) {
        pthread_mutex_lock(&rw->rw_mutex);
    }
    pthread_mutex_unlock(&rw->r_mutex);
    pthread_mutex_unlock(&rw->w_mutex);
}

void ReaderUnlock(struct read_write_lock* rw)
{
    pthread_mutex_lock(&rw->r_mutex);
    --rw->curr_r_cnt;
    if (rw->curr_r_cnt == 0) {
        pthread_mutex_unlock(&rw->rw_mutex);
    }
    pthread_mutex_unlock(&rw->r_mutex);
}

void WriterLock(struct read_write_lock* rw)
{
    pthread_mutex_lock(&rw->w_mutex);
    ++rw->waiting_w_cnt;
    pthread_mutex_unlock(&rw->w_mutex);

    pthread_mutex_lock(&rw->rw_mutex);

    pthread_mutex_lock(&rw->w_mutex);
    --rw->waiting_w_cnt;
    if (rw->waiting_w_cnt == 0) {
        pthread_cond_broadcast(&rw->w_cond);
    }
    pthread_mutex_unlock(&rw->w_mutex);
}

void WriterUnlock(struct read_write_lock* rw)
{
    pthread_mutex_unlock(&rw->rw_mutex);
}

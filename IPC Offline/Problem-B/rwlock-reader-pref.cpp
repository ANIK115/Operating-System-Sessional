#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock* rw)
{
    pthread_mutex_init(&rw->r_mutex, NULL);
    pthread_mutex_init(&rw->rw_mutex, NULL);
    rw->curr_r_cnt = 0;
}

void ReaderLock(struct read_write_lock* rw)
{
    pthread_mutex_lock(&rw->r_mutex);
    ++rw->curr_r_cnt;
    if (rw->curr_r_cnt == 1) {
        pthread_mutex_lock(&rw->rw_mutex);
    }
    pthread_mutex_unlock(&rw->r_mutex);
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
    pthread_mutex_lock(&rw->rw_mutex);
}

void WriterUnlock(struct read_write_lock* rw)
{
    pthread_mutex_unlock(&rw->rw_mutex);
}

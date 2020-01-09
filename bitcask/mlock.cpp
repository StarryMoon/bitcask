#include "mlock.h"

rwlock::rwlock()
{
    refcount     = 0;
    readwaiters  = 0;
    writewaiters = 0;
    pthread_mutex_init(&rw_mutex,NULL);
    pthread_cond_init(&readcond, NULL);
    pthread_cond_init(&writecond, NULL);
}

rwlock::~rwlock()
{
    refcount     = 0;
    readwaiters  = 0;
    writewaiters = 0;
    pthread_mutex_destroy(&rw_mutex);
    pthread_cond_destroy(&readcond);
    pthread_cond_destroy(&writecond);
}

void rwlock::readlock()
{
    pthread_mutex_lock(&rw_mutex);
    while(refcount < 0)
    {
        readwaiters++;
        pthread_cond_wait(&readcond,&rw_mutex);
        readwaiters--;
    }
    refcount++;
    pthread_mutex_unlock(&rw_mutex);
}

void rwlock::writelock()
{
    pthread_mutex_lock(&rw_mutex);
    while(refcount != 0)
    {
        writewaiters++;
        pthread_cond_wait(&writecond,&rw_mutex);
        writewaiters--;
    }
    refcount = -1;
    pthread_mutex_unlock(&rw_mutex);
}

void rwlock::unlock()
{
    pthread_mutex_lock(&rw_mutex);
    if(refcount == -1)
        refcount = 0;
    else
        refcount--;
    if(refcount == 0)
    {
        if(writewaiters > 0)
            pthread_cond_signal(&writecond);
        else if(readwaiters > 0)
            pthread_cond_broadcast(&readcond);
    }
 
    pthread_mutex_unlock(&rw_mutex);
}

int rwlock::tryreadlock()
{
    int ret = 0;
    pthread_mutex_lock(&rw_mutex);
    if(refcount < 0 || writewaiters > 0)
    {
        ret = -1;
    }
    else
        refcount++;
    pthread_mutex_unlock(&rw_mutex);
    return ret;
}

int rwlock::trywritelock()
{
    int ret = 0;
    pthread_mutex_lock(&rw_mutex);
    if(refcount != 0 )
    {
        ret = -1;
    }
    else
        refcount = -1;
    pthread_mutex_unlock(&rw_mutex);
    return ret;
}

/*
class rwlock
{
public:
    rwlock(): read_cnt(0) {
        pthread_mutex_init(&read_mtx, NULL);
        pthread_mutex_init(&write_mtx, NULL);
    }
 
    ~ rwlock() {
        pthread_mutex_destroy(&read_mtx);
        pthread_mutex_destroy(&write_mtx);
    }
 
    void readLock() {
        pthread_mutex_lock(&read_mtx);
        if (++read_cnt == 1)
            pthread_mutex_lock(&write_mtx);
        pthread_mutex_unlock(&read_mtx);
    }
 
    void readUnlock() {
        pthread_mutex_lock(&read_mtx);
        if (--read_cnt == 0)
            pthread_mutex_unlock(&write_mtx);
        pthread_mutex_unlock(&read_mtx);
    }
 
    void writeLock() {
        pthread_mutex_lock(&write_mtx);
    }
 
    void writeUnlock() {
        pthread_mutex_unlock(&write_mtx);
    }
 
private:
    pthread_mutex_t read_mtx;
    pthread_mutex_t write_mtx;
    int read_cnt; // 读锁个数
};
*/
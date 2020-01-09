#include <pthread.h>

class rwlock
{
public:
    rwlock();
    ~rwlock();
    void readlock();
    void writelock();
    void unlock();
    int tryreadlock();
    int trywritelock();
     
private:
    pthread_mutex_t rw_mutex;
    int refcount;   // -1表示有写者，0表示没有加锁，正数表示有多少个读者
    int readwaiters;
    int writewaiters;
    pthread_cond_t readcond;
    pthread_cond_t writecond;
};
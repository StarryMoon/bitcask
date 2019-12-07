#pragma once
#pragma comment(lib, "pthreadVC2.lib")

#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <condition_variable>

typedef struct task_tag {
//    int data;
    int fd;
    uint64_t offset;
    char *content;
    task_tag(int fp, uint64_t off, char *ch) {
        fd = fp;
        offset = off;
        *content = *ch; 
    }
} Task, *PTask;

class MessageQueue {

public:
    MessageQueue();
    ~MessageQueue();

    void PushTask(PTask pTask);
    PTask PopTask();

private:
    std::mutex m_queueMutex;
    std::condition_variable m_cond; 
    std::queue<PTask> m_queue;
};
#pragma once
#pragma comment(lib, "pthreadVC2.lib")

#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <condition_variable>

typedef struct task_tag {
    int data;
    task_tag(int i) : data(i) { }
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
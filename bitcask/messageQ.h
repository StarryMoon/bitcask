#pragma once
#pragma comment(lib, "pthreadVC2.lib")

#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <condition_variable>

typedef struct task_tag {
    int data_fd;
    int hint_fd;
    uint64_t data_offset;
    uint64_t hint_offset;
    char *data_content;
    char *hint_content;

    task_tag(int d_fp, int h_fp, uint64_t d_off, uint64_t h_off, char *d_ch, char *h_ch) {
        data_fd = d_fp;
        hint_fd = h_fp;
        data_offset = d_off;
        hint_offset = h_off;
        *data_content = *d_ch;
        *hint_content = *h_ch;
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
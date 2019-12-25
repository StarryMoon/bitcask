#pragma once
#pragma comment(lib, "pthreadVC2.lib")

#include "messageQ.h"

    MessageQueue::MessageQueue() {}

    MessageQueue::~MessageQueue() {
        if (!m_queue.empty()) {
            PTask pRtn = m_queue.front();
            delete pRtn;
        } 
    }

    void MessageQueue::PushTask(PTask pTask) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_queue.push(pTask);
        m_cond.notify_one();
    }

    PTask MessageQueue::PopTask() {
        PTask pRtn = NULL;
        std::unique_lock<std::mutex> lock(m_queueMutex);
        while (m_queue.empty()) {
            m_cond.wait_for(lock, std::chrono::seconds(1)); // wait > 1s
        }

        if (!m_queue.empty()) {
            pRtn = m_queue.front();
            //if (pRtn->content != NULL) {
                m_queue.pop();
            //}
        }

        return pRtn;
    }

    bool MessageQueue::respondse() {
        std::unique_lock <std::mutex> lck(mtx);
        while (!ready) {
            cv.wait(lck);
        }
        return true;
    }

    void MessageQueue::doAction() {
        std::unique_lock <std::mutex> lck(mtx);
        ready = true;
        cv.notify_all();
    }
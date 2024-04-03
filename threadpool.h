//=================================================================================================
//===
//=== threadpool.h
//===
//=== Copyright (c) 2020-2023 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaly
//===
//=================================================================================================
#ifndef _SIMPLEUTILS_THREADPOOL_H_
#define _SIMPLEUTILS_THREADPOOL_H_
#pragma once

#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>
#include <future>
#include <unordered_set>
#include <atomic>
#include <future>

using TaskID = uint64_t;
using LockGuard = std::lock_guard<std::mutex>;
using LockUnique = std::unique_lock<std::mutex>;

namespace su
{

class ThreadPool
{
public:
    ThreadPool(uint32_t numThreads)
    {
        if (!numThreads)
        {
            numThreads = getMaxThreads();
            if (numThreads >= 2)
            {
                numThreads -= 2;
            }
        }
        m_threads.reserve(numThreads);

        for (uint32_t ii = 0; ii < numThreads; ++ii)
        {
            m_threads.emplace_back(&ThreadPool::run, this);
        }
    }

    virtual ~ThreadPool()
    {
        m_exit = true;

        for (auto& itm : m_threads)
        {
            m_queueCV.notify_all();
            itm.join();
        }
    }

    static uint32_t getMaxThreads()
    {
        return std::thread::hardware_concurrency();
    }

    uint32_t getThreadsCount() const
    {
        return m_threads.size();
    }

    template <typename Func, typename ...Args>
    TaskID add_task(const Func& task_func, Args&&... args)
    {
        LockGuard lock(m_queueMutex);

        TaskID task_idx = m_lastIndex++;

        m_queue.emplace(std::async(std::launch::deferred, task_func, args...), task_idx);
        m_queueCV.notify_one();

        return task_idx;
    }

    void wait(TaskID task_id)
    {
        LockUnique lock(m_completedTaskMutex);

        m_completedTaskCV.wait(lock, [this, task_id]()->bool
        {
            return m_completedTask.find(task_id) != m_completedTask.end();
        });
    }

    void wait_all()
    {
        LockUnique lock(m_queueMutex);

        m_completedTaskCV.wait(lock, [this]()->bool
        {
            std::lock_guard<std::mutex> task_lock(m_completedTaskMutex);
            return m_queue.empty() && m_lastIndex == m_completedTask.size();
        });
    }

    bool isTaskFinished(TaskID task_id)
    {
        LockGuard lock(m_completedTaskMutex);

        if (m_completedTask.find(task_id) != m_completedTask.end())
        {
            return true;
        }
        return false;
    }

private:
    void run()
    {
        while (!m_exit)
        {
            LockUnique lock(m_queueMutex);

            m_queueCV.wait(lock, [this]()->bool { return !m_queue.empty() || m_exit; });

            if (m_exit)
            {
                break;
            }

            if (!m_queue.empty())
            {
                auto elem = std::move(m_queue.front());
                m_queue.pop();
                lock.unlock();

                elem.first.get();

                LockGuard lock_result(m_completedTaskMutex);
                m_completedTask.insert(elem.second);

                m_completedTaskCV.notify_all();
            }
        }
    }

    std::queue<std::pair<std::future<void>, TaskID>> m_queue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;

    std::unordered_set<TaskID> m_completedTask;
    std::mutex m_completedTaskMutex;
    std::condition_variable m_completedTaskCV;

    std::vector<std::thread> m_threads;

    std::atomic<bool> m_exit = false;
    std::atomic<TaskID> m_lastIndex = 0;
};

}

#endif

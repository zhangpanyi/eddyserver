#include "thread_pool.h"
#include <cassert>
#include <iostream>
#include <algorithm>

Thread::Thread()
    : finished_(false)
{
    std::function<void()> worker = std::bind(&Thread::run_loop, this);
    thread_ = std::make_unique<std::thread>(std::move(worker));
}

// 合并线程
void Thread::join()
{
    termminiate();
    thread_->join();
}

// 终止线程
void Thread::termminiate()
{
    if (!finished_)
    {
        finished_ = true;
        condition_incoming_task_.notify_one();
    }
}

// 获取负载
size_t Thread::load() const
{
    size_t count = 0;
    {
        std::lock_guard<std::mutex> lock(queue_task_mutex_);
        count = queue_task_.size();
    }
    return count;
}

// 等待到空闲
void Thread::wait_for_idle()
{
    while (load() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// 添加任务
size_t Thread::append(Callback &&cb)
{
    size_t size = 0;
    if (!finished_)
    {
        {
            std::lock_guard<std::mutex> lock(queue_task_mutex_);
            queue_task_.push_back(std::forward<Callback>(cb));
            size = queue_task_.size();
        }

        if (size == 1)
        {
            condition_incoming_task_.notify_one();
        }
    }
    return size;
}

size_t Thread::append(const Callback &cb)
{
    size_t size = 0;
    if (!finished_)
    {
        {
            std::lock_guard<std::mutex> lock(queue_task_mutex_);
            queue_task_.push_back(cb);
            size = queue_task_.size();
        }

        if (size == 1)
        {
            condition_incoming_task_.notify_one();
        }
    }
    return size;
}

// 线程循环
void Thread::run_loop()
{
    while (!finished_)
    {
        {
            std::unique_lock<std::mutex> lock(queue_task_mutex_);
            while (queue_task_.empty())
            {
                if (finished_)
                {
                    break;
                }
                condition_incoming_task_.wait(lock);
            }
        }

        if (finished_)
        {
            break;
        }

        Callback cb;
        {
            std::lock_guard<std::mutex> lock(queue_task_mutex_);
            if (!queue_task_.empty())
            {
                cb = std::move(queue_task_.front());
            }
        }

        if (cb != nullptr)
        {
            try
            {
                cb();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
            std::lock_guard<std::mutex> lock(queue_task_mutex_);
            queue_task_.pop_front();
        }
    }
}

/************************************************************************/
/************************************************************************/

namespace thread_pool_stuff
{
    struct min_load_cmp
    {
        bool operator() (const ThreadPool::ThreadPointer &lhs,
            const ThreadPool::ThreadPointer &rhs) const
        {
            return lhs->load() < rhs->load();
        }
    };
}

ThreadPool::ThreadPool(size_t thread_num)
{
    assert(thread_num > 0);
    if (thread_num > 0)
    {
        for (size_t i = 0; i < thread_num; ++i)
        {
            vector_thread_.push_back(std::make_shared<Thread>());
        }
    }
}

// 获取线程数量
size_t ThreadPool::count() const
{
    return vector_thread_.size();
}

// 合并线程
void ThreadPool::join()
{
    for (size_t i = 0; i < vector_thread_.size(); ++i)
    {
        vector_thread_[i]->join();
    }
}

// 终止线程
void ThreadPool::termminiate()
{
    for (size_t i = 0; i < vector_thread_.size(); ++i)
    {
        vector_thread_[i]->termminiate();
    }
}

// 获取负载
size_t ThreadPool::load(size_t index) const
{
    return index < vector_thread_.size() ? vector_thread_[index]->load() : 0;
}

// 等待到空闲
void ThreadPool::wait_for_idle()
{
    for (size_t i = 0; i < vector_thread_.size(); ++i)
    {
        vector_thread_[i]->wait_for_idle();
    }
}

// 添加任务
void ThreadPool::append(Thread::Callback &&task)
{
    assert(!vector_thread_.empty());
    if (!vector_thread_.empty())
    {
        std::vector<ThreadPointer>::iterator itr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            itr = std::min_element(vector_thread_.begin(), vector_thread_.end(), thread_pool_stuff::min_load_cmp());
        }
        (*itr)->append(std::forward<Thread::Callback>(task));
    }
}

void ThreadPool::append(const Thread::Callback &task)
{
    assert(!vector_thread_.empty());
    if (!vector_thread_.empty())
    {
        std::vector<ThreadPointer>::iterator itr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            itr = std::min_element(vector_thread_.begin(), vector_thread_.end(), thread_pool_stuff::min_load_cmp());
        }
        (*itr)->append(task);
    }
}

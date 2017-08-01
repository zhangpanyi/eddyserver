#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <list>
#include <mutex>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>

class Thread final
{
public:
    typedef std::function<void()> Callback;
    typedef std::unique_ptr<std::thread> ThreadPointer;

public:
    Thread();

public:
    /**
     * 合并线程
     */
    void join();

    /**
     * 终止线程
     */
    void termminiate();

    /**
     * 获取负载
     */
    size_t load() const;

    /**
     * 等待到空闲
     */
    void wait_for_idle();

    /**
     * 添加任务
     */
    size_t append(Callback &&cb);
    size_t append(const Callback &cb);

private:
    /**
     * 线程循环
     */
    void run_loop();

private:
    Thread(const Thread&) = delete;
    Thread& operator= (const Thread&) = delete;

private:
    std::atomic_bool        finished_;
    ThreadPointer           thread_;
    std::list<Callback>     queue_task_;
    mutable std::mutex      queue_task_mutex_;
    std::condition_variable condition_incoming_task_;
};

class ThreadPool final
{
public:
    typedef std::shared_ptr<Thread> ThreadPointer;

public:
    ThreadPool(size_t thread_num);

public:
    /**
     * 获取线程数量
     */
    size_t count() const;

    /**
     * 合并线程
     */
    void join();

    /**
     * 终止线程
     */
    void termminiate();

    /**
     * 获取负载
     */
    size_t load(size_t index) const;

    /**
     * 等待到空闲
     */
    void wait_for_idle();

    /**
     * 添加任务
     */
    void append(Thread::Callback &&cb);
    void append(const Thread::Callback &cb);

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator= (const ThreadPool&) = delete;

private:
    mutable std::mutex          mutex_;
    std::vector<ThreadPointer>  vector_thread_;
};

#endif

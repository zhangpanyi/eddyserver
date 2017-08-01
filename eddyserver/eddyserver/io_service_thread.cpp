#include "io_service_thread.h"
#include <chrono>
#include <iostream>
#include "tcp_session.h"
#include "io_service_thread_manager.h"

namespace eddyserver
{
    IOServiceThread::IOServiceThread(IOThreadID id, IOServiceThreadManager &td_manager)
        : td_id_(id)
        , timer_(io_service_)
        , td_manager_(td_manager)
        , wait_handler_(std::bind(&IOServiceThread::check_keep_alive, this, std::placeholders::_1))
    {
    }

    // 线程执行函数
    void IOServiceThread::run()
    {
        if (io_work_ == nullptr)
        {
            io_work_ = std::make_unique<asio::io_service::work>(io_service_);
        }

        timer_.expires_from_now(std::chrono::seconds(1));
        timer_.async_wait(wait_handler_);

        asio::error_code error_code;
        io_service_.run(error_code);
        if (error_code)
        {
            std::cerr << error_code.message() << std::endl;
        }
    }

    // 合并线程
    void IOServiceThread::join()
    {
        if (thread_ != nullptr)
        {
            thread_->join();
        }
    }

    // 停止线程
    void IOServiceThread::stop()
    {
        if (io_work_ != nullptr)
        {
            io_work_.reset();
        }
    }

    // 运行线程
    void IOServiceThread::run_thread()
    {
        if (thread_ == nullptr)
        {
            std::function<void()> cb = std::bind(&IOServiceThread::run, this);
            thread_ = std::make_unique<std::thread>(std::move(cb));
        }
    }

    // 检查Session存活
    void IOServiceThread::check_keep_alive(asio::error_code error_code)
    {
        if (error_code)
        {
            std::cerr << error_code.message() << std::endl;
        }
        else
        {
            session_queue_.foreach([=](const SessionPointer &session)
            {
                if (!session->check_keep_alive())
                {
                    io_service_.post(std::bind(&TCPSession::close, session));
                }
            });
            timer_.expires_from_now(std::chrono::seconds(1));
            timer_.async_wait(wait_handler_);
        }
    }
}

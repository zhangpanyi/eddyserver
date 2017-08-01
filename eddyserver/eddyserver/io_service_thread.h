#ifndef __IO_SERVICE_THREAD_H__
#define __IO_SERVICE_THREAD_H__

#include <memory>
#include <thread>
#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include "tcp_session_queue.h"

namespace eddyserver
{
    class IOServiceThreadManager;

    class IOServiceThread final : public std::enable_shared_from_this< IOServiceThread >
    {
        friend class IOServiceThreadManager;
        typedef std::function<void(asio::error_code error_code)> WaitHandler;

    public:
        IOServiceThread(IOThreadID id, IOServiceThreadManager &td_manager);

    public:
        /**
         * 合并线程
         */
        void join();

        /**
         * 停止线程
         */
        void stop();

        /**
         * 运行线程
         */
        void run_thread();

        /**
         * 投递请求
         */
        template <typename CompletionHandler>
        void post(ASIO_MOVE_ARG(CompletionHandler) handler)
        {
            io_service_.post(handler);
        }

    public:
        /**
         * 获取线程id
         */
        IOThreadID get_id() const
        {
            return td_id_;
        }

        /**
         * 获取Session队列
         * 此线程管辖的所有Session存放在此
         */
        TCPSessionQueue& get_session_queue()
        {
            return session_queue_;
        }

        /**
         * 获取io_service
         */
        asio::io_service& get_io_service()
        {
            return io_service_;
        }

        /**
         * 获取线程管理器
         */
        IOServiceThreadManager& get_thread_manager()
        {
            return td_manager_;
        }

    private:
        /**
         * 线程执行函数
         */
        void run();

        /**
         * 检查Session存活
         */
        void check_keep_alive(asio::error_code error_code);

    private:
        IOServiceThread(const IOServiceThread&) = delete;
        IOServiceThread& operator= (const IOServiceThread&) = delete;

    private:
        const IOThreadID                        td_id_;
        IOServiceThreadManager&                 td_manager_;
        asio::io_service                        io_service_;
        asio::steady_timer                      timer_;
        const WaitHandler                       wait_handler_;
        std::unique_ptr<std::thread>            thread_;
        std::unique_ptr<asio::io_service::work> io_work_;
        TCPSessionQueue                         session_queue_;
    };
}

#endif

#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <asio.hpp>
#include "types.h"

namespace eddyserver
{
    class IOServiceThreadManager;

    class TCPClient final
    {
    public:
        TCPClient(IOServiceThreadManager &io_thread_manager,
            const SessionHandlerCreator &handler_creator,
            const MessageFilterCreator &filter_creator);

    public:
        /**
         * 发起连接请求
         */
        void connect(asio::ip::tcp::endpoint &endpoint,
            asio::error_code &error_code);

        /**
         * 发起异步连接请求
         */
        void async_connect(asio::ip::tcp::endpoint &endpoint,
            const std::function<void(asio::error_code)> &cb);

    private:
        /**
         * 处理连接结果
         */
        void handle_connect(SessionPointer session_ptr,
            asio::error_code error_code);

        /**
         * 异步连接结果
         */
        void handle_async_connect(SessionPointer session_ptr,
            std::function<void(asio::error_code)> cb,
            asio::error_code error_code);

    private:
        TCPClient(const TCPClient&) = delete;
        TCPClient& operator= (const TCPClient&) = delete;

    private:
        IOServiceThreadManager& io_thread_manager_;
        SessionHandlerCreator   session_handler_creator_;
        MessageFilterCreator    message_filter_creator_;
    };
}

#endif

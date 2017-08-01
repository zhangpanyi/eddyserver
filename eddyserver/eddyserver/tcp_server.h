#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <asio.hpp>
#include "types.h"

namespace eddyserver
{
    class IOServiceThreadManager;

    class TCPServer final
    {
    public:
        TCPServer(asio::ip::tcp::endpoint &endpoint,
            IOServiceThreadManager &io_thread_manager,
            const SessionHandlerCreator &handler_creator,
            const MessageFilterCreator &filter_creator,
            uint32_t keep_alive_time = 0);

    public:
        /**
         * 获取本地端点信息
         */
        asio::ip::tcp::endpoint get_local_endpoint() const
        {
            return acceptor_.local_endpoint();
        }

    private:
        /**
         * 处理接受事件
         */
        void handle_accept(SessionPointer session_ptr, asio::error_code error_code);

    private:
        TCPServer(const TCPServer&) = delete;
        TCPServer& operator= (const TCPServer&) = delete;

    private:
        const uint32_t          keep_alive_time_;
        asio::ip::tcp::acceptor acceptor_;
        IOServiceThreadManager& io_thread_manager_;
        SessionHandlerCreator   session_handler_creator_;
        MessageFilterCreator    message_filter_creator_;
    };
}

#endif

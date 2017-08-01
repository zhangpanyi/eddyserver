#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

#include <chrono>
#include <asio/ip/tcp.hpp>
#include "types.h"
#include "net_message.h"

namespace eddyserver
{
    class TCPSession final : public std::enable_shared_from_this< TCPSession >
    {
        friend class IOServiceThread;
        friend class IOServiceThreadManager;

        typedef asio::ip::tcp::socket SocketType;
        typedef std::chrono::steady_clock::time_point TimePoint;

    public:
        TCPSession(ThreadPointer &td, MessageFilterPointer &filter, uint32_t keep_alive_time = 0);

    public:
        /**
         * 获取socket
         */
        SocketType& get_socket()
        {
            return socket_;
        }

        /**
         * 获取Session ID
         */
        TCPSessionID get_id() const
        {
            return session_id_;
        }

        /**
         * 获取线程
         */
        ThreadPointer& get_io_thread()
        {
            return io_thread_;
        }

        /**
         * 获取收到的消息列表
         */
        std::vector<NetMessage>& get_messages_received()
        {
            return messages_received_;
        }

        /**
         * 投递消息列表
         */
        void post_message_list(const std::vector<NetMessage> &messages);

        /**
         * 关闭Session
         */
        void close();

    private:
        /**
         * 初始化
         */
        void init(TCPSessionID id);

        /**
         * 检查Session存活
         */
        bool check_keep_alive();

    private:
        /**
         * 处理读
         */
        void handle_read(asio::error_code error_code, size_t bytes_transferred);

        /**
         * 处理写
         */
        void hanlde_write(asio::error_code error_code, size_t bytes_transferred);

        /**
         * 处理关闭
         */
        void hanlde_close();

        /**
         * 处理安全关闭
         */
        void handle_safe_close(asio::error_code error_code, size_t bytes_transferred);

    private:
        TCPSession(const TCPSession&) = delete;
        TCPSession& operator= (const TCPSession&) = delete;

    private:
        bool                        closed_;
        int                         num_read_handlers_;
        int                         num_write_handlers_;
        TCPSessionID                session_id_;
        SocketType                  socket_;
        ThreadPointer               io_thread_;
        asio::steady_timer          close_timer_;
        MessageFilterPointer        msg_filter_;
        TimePoint                   last_activity_time_;
        std::vector<uint8_t>        buffer_receiving_;
        std::vector<uint8_t>        buffer_sending_;
        std::vector<uint8_t>        buffer_to_be_sent_;
        NetMessageVector            messages_received_;
        const std::chrono::seconds  keep_alive_time_;
    };
}

#endif

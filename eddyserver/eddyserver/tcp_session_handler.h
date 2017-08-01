#ifndef __TCP_SESSION_HANDLE_H__
#define __TCP_SESSION_HANDLE_H__

#include <vector>
#include <asio/ip/tcp.hpp>
#include "types.h"
#include "net_message.h"

namespace eddyserver
{
    class IOServiceThreadManager;

    class TCPSessionHandler : public std::enable_shared_from_this < TCPSessionHandler >
    {
        friend class IOServiceThreadManager;

    public:
        TCPSessionHandler();
        virtual ~TCPSessionHandler() = default;

    public:
        /**
         * 连接事件
         */
        virtual void on_connected() = 0;

        /**
         * 接收消息事件
         */
        virtual void on_message(NetMessage &message) = 0;

        /**
         * 关闭事件
         */
        virtual void on_closed() = 0;

    public:
        /**
         * 是否已关闭
         */
        bool is_closed() const
        {
            return session_id_ == 0;
        }

        /**
         * 获取线程id
         */
        IOThreadID get_thread_id() const
        {
            return thread_id_;
        }

        /**
         * 获取Session ID
         */
        TCPSessionID get_session_id() const
        {
            return session_id_;
        }

        /**
         * 获取线程管理器
         */
        IOServiceThreadManager* get_thread_manager()
        {
            return io_thread_manager_;
        }

        /**
         * 获取将被发送的消息列表
         */ 
        std::vector<NetMessage>& messages_to_be_sent()
        {
            return messages_to_be_sent_;
        }

        /**
         * 获取对端端点信息
         */
        const asio::ip::tcp::endpoint& remote_endpoint() const
        {
            return remote_endpoint_;
        }

    public:
        /**
         * 发送消息
         */
        void send(const NetMessage &message);

        /**
         * 关闭连接
         */
        void close();

        /**
         * 处置连接
         */
        void dispose();

    private:
        /**
         * 初始化
         */
        void init(TCPSessionID sid,
            IOThreadID tid,
            IOServiceThreadManager *manager,
            const asio::ip::tcp::endpoint &remote_endpoint);

    private:
        TCPSessionHandler(const TCPSessionHandler&) = delete;
        TCPSessionHandler& operator= (const TCPSessionHandler&) = delete;

    private:
        TCPSessionID            session_id_;
        IOThreadID              thread_id_;
        asio::ip::tcp::endpoint remote_endpoint_;
        IOServiceThreadManager* io_thread_manager_;
        std::vector<NetMessage>     messages_to_be_sent_;
    };
}

#endif

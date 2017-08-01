#include "tcp_session.h"
#include <iostream>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include "message_filter.h"
#include "io_service_thread.h"
#include "tcp_session_handler.h"
#include "io_service_thread_manager.h"

namespace eddyserver
{
    namespace session_stuff
    {
        typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

        /**
         * 发送消息列表到SessionHandler
         */
        void SendMessageListToHandler(IOServiceThreadManager &manager,
            TCPSessionID id,
            NetMessageVecPointer messages_received)
        {
            SessionHandlePointer handler_ptr = manager.get_session_handler(id);
            if (handler_ptr != nullptr)
            {
                for (size_t i = 0; i < messages_received->size(); ++i)
                {
                    handler_ptr->on_message(messages_received->at(i));
                }
            }
        }

        /**
         * 投递消息列表到主线程操作
         */
        void PackMessageList(SessionPointer session_ptr)
        {
            if (!session_ptr->get_messages_received().empty())
            {
                NetMessageVecPointer messages_received = std::make_shared< std::vector<NetMessage> >();
                *messages_received = std::move(session_ptr->get_messages_received());
                session_ptr->get_io_thread()->get_thread_manager().get_main_thread()->post(std::bind(
                    SendMessageListToHandler,
                    std::ref(session_ptr->get_io_thread()->get_thread_manager()),
                    session_ptr->get_id(),
                    messages_received
                ));
            }
        }

        /**
         * 直接发送消息列表到SessionHandler
         */
        void SendMessageListDirectly(SessionPointer session_ptr)
        {
            SessionHandlePointer handler_ptr = session_ptr->get_io_thread()->get_thread_manager().get_session_handler(session_ptr->get_id());
            if (handler_ptr != nullptr)
            {
                std::vector<NetMessage> &messages_received = session_ptr->get_messages_received();
                for (size_t i = 0; i < messages_received.size(); ++i)
                {
                    handler_ptr->on_message(messages_received[i]);
                }
                messages_received.clear();
            }
        }
    }

    TCPSession::TCPSession(ThreadPointer &td, MessageFilterPointer &filter, uint32_t keep_alive_time)
        : closed_(true)
        , session_id_(0)
        , io_thread_(td)
        , msg_filter_(filter)
        , num_read_handlers_(0)
        , num_write_handlers_(0)
        , socket_(td->get_io_service())
        , keep_alive_time_(keep_alive_time)
        , close_timer_(td->get_io_service())
    {
    }

    // 初始化
    void TCPSession::init(TCPSessionID id)
    {
        assert(id > 0);

        closed_ = false;
        session_id_ = id;
        SessionPointer self = shared_from_this();
        io_thread_->get_session_queue().add(self);
        last_activity_time_ = std::chrono::steady_clock::now();

        asio::ip::tcp::no_delay option(true);
        socket_.set_option(option);

        size_t bytes_wanna_read = msg_filter_->bytes_wanna_read();
        if (bytes_wanna_read == 0)
        {
            return;
        }

        ++num_read_handlers_;
        if (bytes_wanna_read == MessageFilterInterface::any_bytes())
        {
            buffer_receiving_.resize(NetMessage::kDynamicThreshold);
            socket_.async_read_some(asio::buffer(buffer_receiving_.data(), buffer_receiving_.size()),
                std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            buffer_receiving_.resize(bytes_wanna_read);
            asio::async_read(socket_, asio::buffer(buffer_receiving_.data(), bytes_wanna_read),
                std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }

    // 处理关闭
    void TCPSession::hanlde_close()
    {
        if (num_write_handlers_ > 0)
        {
            return;
        }

        if (io_thread_->get_session_queue().get(get_id()) != nullptr)
        {
            io_thread_->get_thread_manager().get_main_thread()->post(
                std::bind(&IOServiceThreadManager::on_session_closed, &io_thread_->get_thread_manager(), get_id()));

            asio::error_code error_code;
            socket_.shutdown(asio::ip::tcp::socket::shutdown_send, error_code);
            if (error_code && error_code != asio::error::not_connected)
            {
                std::cerr << error_code.message() << std::endl;
            }
            io_thread_->get_session_queue().remove(get_id());

            buffer_receiving_.resize(NetMessage::kDynamicThreshold);
            socket_.async_read_some(asio::buffer(buffer_receiving_.data(), buffer_receiving_.size()),
                std::bind(&TCPSession::handle_safe_close, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

            close_timer_.expires_from_now(std::chrono::seconds(5));
            close_timer_.async_wait([=](asio::error_code error)
            {
                if (error != asio::error::operation_aborted)
                {
                    socket_.cancel();
                    socket_.close();
                }
            });
        }
    }

    // 关闭会话
    void TCPSession::close()
    {
        if (closed_)
        {
            return;
        }
        closed_ = true;
        hanlde_close();
    }

    // 检查Session存活
    bool TCPSession::check_keep_alive()
    {
        if (keep_alive_time_.count() == 0)
        {
            return true;
        }
        return std::chrono::steady_clock::now() - last_activity_time_ < keep_alive_time_;
    }

    // 投递消息列表
    void TCPSession::post_message_list(const std::vector<NetMessage> &messages)
    {
        if (closed_ || messages.empty())
        {
            return;
        }

        size_t bytes_wanna_write = msg_filter_->bytes_wanna_write(messages);
        if (bytes_wanna_write == 0)
        {
            return;
        }

        buffer_to_be_sent_.reserve(buffer_to_be_sent_.size() + bytes_wanna_write);
        msg_filter_->write(messages, buffer_to_be_sent_);

        if (buffer_sending_.empty())
        {
            ++num_write_handlers_;
            buffer_sending_.swap(buffer_to_be_sent_);
            asio::async_write(socket_, asio::buffer(buffer_sending_.data(), bytes_wanna_write),
                std::bind(&TCPSession::hanlde_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }

    // 处理读
    void TCPSession::handle_read(asio::error_code error_code, size_t bytes_transferred)
    {
        --num_read_handlers_;
        assert(num_read_handlers_ >= 0);

        if (error_code || closed_)
        {
            closed_ = true;
            hanlde_close();
            return;
        }

        bool wanna_post = messages_received_.empty();
        size_t bytes_read = msg_filter_->read(buffer_receiving_, messages_received_);
        assert(bytes_read == bytes_transferred);
        if (bytes_read != bytes_transferred)
        {
            std::cerr << "bytes_read: " << bytes_read << " bytes_transferred: " << bytes_transferred << std::endl;
        }

        buffer_receiving_.clear();
        wanna_post = wanna_post && !messages_received_.empty();

        if (wanna_post)
        {
            if (io_thread_->get_id() == io_thread_->get_thread_manager().get_main_thread()->get_id())
            {
                io_thread_->post(std::bind(session_stuff::SendMessageListDirectly, shared_from_this()));
            }
            else
            {
                io_thread_->post(std::bind(session_stuff::PackMessageList, shared_from_this()));
            }
            last_activity_time_ = std::chrono::steady_clock::now();
        }

        size_t bytes_wanna_read = msg_filter_->bytes_wanna_read();
        if (bytes_wanna_read == 0)
        {
            return;
        }

        ++num_read_handlers_;
        if (bytes_wanna_read == MessageFilterInterface::any_bytes())
        {
            buffer_receiving_.resize(NetMessage::kDynamicThreshold);
            socket_.async_read_some(asio::buffer(&*buffer_receiving_.begin(), buffer_receiving_.size()),
                std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            buffer_receiving_.resize(bytes_wanna_read);
            asio::async_read(socket_, asio::buffer(buffer_receiving_.data(), bytes_wanna_read),
                std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }

    // 处理写
    void TCPSession::hanlde_write(asio::error_code error_code, size_t bytes_transferred)
    {
        --num_write_handlers_;
        assert(num_write_handlers_ >= 0);

        if (error_code || closed_)
        {
            closed_ = true;
            hanlde_close();
            return;
        }

        buffer_sending_.clear();

        if (buffer_to_be_sent_.empty())
        {
            return;
        }

        ++num_write_handlers_;
        buffer_sending_.swap(buffer_to_be_sent_);
        asio::async_write(socket_, asio::buffer(buffer_sending_.data(), buffer_sending_.size()),
            std::bind(&TCPSession::hanlde_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    // 处理安全关闭
    void TCPSession::handle_safe_close(asio::error_code error_code, size_t bytes_transferred)
    {
        if (bytes_transferred == 0)
        {        
            if (error_code != asio::error::operation_aborted)
            {
                close_timer_.cancel();
                socket_.close();
            }        
        }
        else
        {
            buffer_receiving_.resize(NetMessage::kDynamicThreshold);
            socket_.async_read_some(asio::buffer(buffer_receiving_.data(), buffer_receiving_.size()),
                std::bind(&TCPSession::handle_safe_close, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }
}

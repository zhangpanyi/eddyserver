#include "tcp_server.h"
#include <iostream>
#include "tcp_session.h"
#include "io_service_thread.h"
#include "tcp_session_handler.h"
#include "io_service_thread_manager.h"

namespace eddyserver
{
    TCPServer::TCPServer(asio::ip::tcp::endpoint &endpoint,
        IOServiceThreadManager &io_thread_manager,
        const SessionHandlerCreator &handler_creator,
        const MessageFilterCreator &filter_creator,
        uint32_t keep_alive_time)
        : keep_alive_time_(keep_alive_time)
        , io_thread_manager_(io_thread_manager)
        , session_handler_creator_(handler_creator)
        , message_filter_creator_(filter_creator)
        , acceptor_(io_thread_manager.get_main_thread()->get_io_service(), endpoint)
    {
        MessageFilterPointer filter_ptr = message_filter_creator_();
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        SessionPointer session_ptr = std::make_shared<TCPSession>(
            io_thread_manager_.get_min_load_thread(), filter_ptr, keep_alive_time_);
        acceptor_.async_accept(session_ptr->get_socket(),
            std::bind(&TCPServer::handle_accept, this, session_ptr, std::placeholders::_1));
    }

    // 处理接受事件
    void TCPServer::handle_accept(SessionPointer session_ptr, asio::error_code error_code)
    {
        if (error_code)
        {
            std::cerr << error_code.message() << std::endl;
            assert(false);
            return;
        }

        SessionHandlePointer handle_ptr = session_handler_creator_();
        io_thread_manager_.on_session_connected(session_ptr, handle_ptr);

        ThreadPointer td = io_thread_manager_.get_min_load_thread();
        MessageFilterPointer filter_ptr = message_filter_creator_();
        SessionPointer new_session_ptr = std::make_shared<TCPSession>(
            td, filter_ptr, keep_alive_time_);
        acceptor_.async_accept(new_session_ptr->get_socket(),
            std::bind(&TCPServer::handle_accept, this, new_session_ptr, std::placeholders::_1));
    }
}

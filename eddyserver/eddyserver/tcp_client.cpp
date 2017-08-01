#include "tcp_client.h"
#include <iostream>
#include "tcp_session.h"
#include "io_service_thread.h"
#include "tcp_session_handler.h"
#include "io_service_thread_manager.h"

namespace eddyserver
{
	TCPClient::TCPClient(IOServiceThreadManager &io_thread_manager,
		const SessionHandlerCreator &handler_creator,
		const MessageFilterCreator &filter_creator)
		: io_thread_manager_(io_thread_manager)
		, session_handler_creator_(handler_creator)
		, message_filter_creator_(filter_creator)
	{
	}

    // 发起连接请求
    void TCPClient::connect(asio::ip::tcp::endpoint &endpoint,
        asio::error_code &error_code)
	{
		MessageFilterPointer filter_ptr = message_filter_creator_();
        SessionPointer session_ptr = std::make_shared<TCPSession>(
            io_thread_manager_.get_min_load_thread(), filter_ptr);
		session_ptr->get_socket().connect(endpoint, error_code);
        handle_connect(session_ptr, error_code);
	}

    // 发起异步连接请求
    void TCPClient::async_connect(asio::ip::tcp::endpoint &endpoint,
        const std::function<void(asio::error_code)> &cb)
	{
		MessageFilterPointer filter_ptr = message_filter_creator_();
        SessionPointer session_ptr = std::make_shared<TCPSession>(
            io_thread_manager_.get_min_load_thread(), filter_ptr);
        session_ptr->get_socket().async_connect(endpoint,
            std::bind(&TCPClient::handle_async_connect, this, session_ptr, cb, std::placeholders::_1));
	}

    // 处理连接结果
    void TCPClient::handle_connect(SessionPointer session_ptr,
        asio::error_code error_code)
	{
		if (error_code)
		{
			std::cerr << error_code.message() << std::endl;
			return;
		}

		SessionHandlePointer handle_ptr = session_handler_creator_();
		io_thread_manager_.on_session_connected(session_ptr, handle_ptr);
	}

    // 异步连接结果
    void TCPClient::handle_async_connect(SessionPointer session_ptr,
        std::function<void(asio::error_code)> cb,
        asio::error_code error_code)
	{
        handle_connect(session_ptr, error_code);
        if (cb != nullptr)
        {
            cb(error_code);
        }   
	}
}

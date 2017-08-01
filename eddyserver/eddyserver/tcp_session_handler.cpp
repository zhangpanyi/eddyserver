#include "tcp_session_handler.h"
#include "net_message.h"
#include "tcp_session.h"
#include "io_service_thread.h"
#include "io_service_thread_manager.h"

namespace eddyserver
{
	namespace session_handler_stuff
	{
		typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

        /**
         * 关闭Session
         */
		void CloseSession(ThreadPointer thread_ptr, TCPSessionID id)
		{
			SessionPointer session_ptr = thread_ptr->get_session_queue().get(id);
			if (session_ptr != nullptr)
			{
				session_ptr->close();
			}
		}

        /**
         * 发送消息列表到Session
         */
		void SendMessageListToSession(ThreadPointer thread_ptr, TCPSessionID id, NetMessageVecPointer messages)
		{
			SessionPointer session_ptr = thread_ptr->get_session_queue().get(id);
			if (session_ptr != nullptr)
			{
				session_ptr->post_message_list(*messages);
			}
		}

        /**
         * 投递消息列表到线程操作
         */
		void PackMessageList(SessionHandlePointer session_handle_ptr)
		{
			if (!session_handle_ptr->messages_to_be_sent().empty())
			{
                ThreadPointer thread_ptr = session_handle_ptr->get_thread_manager()->get_thread(session_handle_ptr->get_thread_id());
                if (thread_ptr != nullptr)
                {
                    NetMessageVecPointer messages_to_be_sent = std::make_shared< std::vector<NetMessage> >();
                    *messages_to_be_sent = std::move(session_handle_ptr->messages_to_be_sent());
                    thread_ptr->post(std::bind(
                        SendMessageListToSession, thread_ptr, session_handle_ptr->get_session_id(), messages_to_be_sent));
                }
			}
		}

        /**
         * 直接发送消息列表
         */
		void SendMessageListDirectly(SessionHandlePointer session_handle_ptr)
		{
			ThreadPointer thread_ptr = session_handle_ptr->get_thread_manager()->get_thread(session_handle_ptr->get_thread_id());
			if (thread_ptr != nullptr)
			{
				SessionPointer session_ptr = thread_ptr->get_session_queue().get(session_handle_ptr->get_session_id());
				if (session_ptr != nullptr)
				{
					session_ptr->post_message_list(session_handle_ptr->messages_to_be_sent());
					session_handle_ptr->messages_to_be_sent().clear();
				}
			}
		}
	}

	TCPSessionHandler::TCPSessionHandler()
		: session_id_(0)
	{
	}

    // 初始化
	void TCPSessionHandler::init(TCPSessionID sid,
        IOThreadID tid,
        IOServiceThreadManager *manager,
        const asio::ip::tcp::endpoint &remote_endpoint)
	{
		thread_id_ = tid;
		session_id_ = sid;
		io_thread_manager_ = manager;
		remote_endpoint_ = remote_endpoint;
	}

    // 处置连接
	void TCPSessionHandler::dispose()
	{
        session_id_ = 0;
	}

    // 关闭连接
	void TCPSessionHandler::close()
	{
		if (is_closed())
		{
			return;
		}

        session_handler_stuff::PackMessageList(shared_from_this());

		ThreadPointer thread_ptr = get_thread_manager()->get_thread(thread_id_);
		if (thread_ptr != nullptr)
		{
			thread_ptr->post(std::bind(session_handler_stuff::CloseSession, thread_ptr, session_id_));
		}
	}

    //  发送消息
	void TCPSessionHandler::send(const NetMessage &message)
	{
		if (is_closed())
		{
			return;
		}

		if (message.empty())
		{
			return;
		}

		bool wanna_send = messages_to_be_sent_.empty();
		messages_to_be_sent_.push_back(message);

		if (wanna_send)
		{
			if (thread_id_ == io_thread_manager_->get_main_thread()->get_id())
			{
				io_thread_manager_->get_main_thread()->post(
                    std::bind(session_handler_stuff::SendMessageListDirectly, shared_from_this()));
			}
			else
			{
				io_thread_manager_->get_main_thread()->post(
                    std::bind(session_handler_stuff::PackMessageList, shared_from_this()));
			}
		}
	}
}

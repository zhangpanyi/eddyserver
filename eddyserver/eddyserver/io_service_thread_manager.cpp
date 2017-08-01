#include "io_service_thread_manager.h"
#include <limits>
#include <cassert>
#include <iostream>
#include "tcp_session.h"
#include "io_service_thread.h"
#include "tcp_session_handler.h"

namespace eddyserver
{
    static const size_t kMainThreadIndex = 0;
    static_assert(kMainThreadIndex == 0, "kMainThreadIndex must be greater than 0");

    IOServiceThreadManager::IOServiceThreadManager(size_t thread_num)
        : id_generator_(1)
    {
        assert(thread_num > kMainThreadIndex);
        if (thread_num == kMainThreadIndex)
        {
            throw std::runtime_error("thread number can not be less than 1!");
        }

        threads_.resize(thread_num);
        thread_load_.resize(thread_num);
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            threads_[i] = std::make_shared<IOServiceThread>(i + 1, *this);
        }
    }

    IOServiceThreadManager::~IOServiceThreadManager()
    {
        stop();
    }

    // 运行线程
    void IOServiceThreadManager::run()
    {
        if (threads_.empty())
        {
            return;
        }

        for (size_t i = 0; i < threads_.size(); ++i)
        {
            if (i != kMainThreadIndex)
            {
                threads_[i]->run_thread();
            }
        }
        threads_[kMainThreadIndex]->run();
    }

    // 停止线程
    void IOServiceThreadManager::stop()
    {
        if (threads_.empty())
        {
            return;
        }

        for (size_t i = 0; i < threads_.size(); ++i)
        {
            if (i != kMainThreadIndex)
            {
                threads_[i]->stop();
            }
        }

        for (size_t i = 0; i < threads_.size(); ++i)
        {
            if (i != kMainThreadIndex)
            {
                threads_[i]->join();
            }
        }

        threads_[kMainThreadIndex]->stop();
    }

    // 获取主线程
    ThreadPointer& IOServiceThreadManager::get_main_thread()
    {
        return threads_[kMainThreadIndex];
    }

    // 获取负载最小的线程
    ThreadPointer& IOServiceThreadManager::get_min_load_thread()
    {
        if (threads_.size() == 1)
        {
            return threads_[kMainThreadIndex];
        }

        size_t min_load_index = kMainThreadIndex;
        size_t min_load_value = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < thread_load_.size(); ++i)
        {
            if (i != kMainThreadIndex && thread_load_[i] < min_load_value)
            {
                min_load_index = i;
            }
        }
        return threads_[min_load_index];
    }

    // 根据线程id获取线程
    ThreadPointer IOServiceThreadManager::get_thread(IOThreadID id)
    {
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            if (threads_[i]->get_id() == id)
            {
                return threads_[i];
            }
        }
        return ThreadPointer();
    }

    // Session连接
    void IOServiceThreadManager::on_session_connected(SessionPointer &session_ptr, SessionHandlePointer &handler_ptr)
    {
        uint32_t session_id = 0;
        if (!id_generator_.get(session_id))
        {
            throw std::runtime_error("generator session id fail!");
        }

        handler_ptr->init(session_id,
            session_ptr->get_io_thread()->get_id(),
            this,
            session_ptr->get_socket().remote_endpoint());

        session_handler_map_.insert(std::make_pair(session_id, handler_ptr));
        session_ptr->get_io_thread()->post(std::bind(&TCPSession::init, session_ptr, session_id));
        handler_ptr->on_connected();

        IOThreadID tid = handler_ptr->get_thread_id();
        assert(tid > 0 && tid <= thread_load_.size());
        if (tid > 0 && tid <= thread_load_.size())
        {
            ++thread_load_[tid - 1];
        }
    }

    // Session关闭
    void IOServiceThreadManager::on_session_closed(TCPSessionID id)
    {
        assert(id > 0);
        SessionHandlerMap::iterator found = session_handler_map_.find(id);
        if (found != session_handler_map_.end())
        {
            SessionHandlePointer handler_ptr = found->second;
            IOThreadID tid = handler_ptr->get_thread_id();
            if (handler_ptr != nullptr)
            {
                handler_ptr->on_closed();
                handler_ptr->dispose();
            }
            session_handler_map_.erase(found);

            assert(tid > 0 && tid <= thread_load_.size());
            if (tid > 0 && tid <= thread_load_.size())
            {
                assert(thread_load_[tid - 1] > 0);
                if (thread_load_[tid - 1] > 0)
                {
                    --thread_load_[tid - 1];
                }
            }
        }

        if (id > 0)
        {
            id_generator_.put(id);
        }
    }

    // 获取Session数量
    size_t IOServiceThreadManager::get_session_count() const
    {
        return session_handler_map_.size();
    }

    // 获取Session处理器
    SessionHandlePointer IOServiceThreadManager::get_session_handler(TCPSessionID id) const
    {
        SessionHandlerMap::const_iterator found = session_handler_map_.find(id);
        if (found != session_handler_map_.end())
        {
            return found->second;
        }
        return SessionHandlePointer();
    }
}

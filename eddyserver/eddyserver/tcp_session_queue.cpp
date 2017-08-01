#include "tcp_session_queue.h"
#include "tcp_session.h"

namespace eddyserver
{
    // 获取Session数量
    size_t TCPSessionQueue::size() const
    {
        return session_queue_.size();
    }

    // 添加Session入队列
    void TCPSessionQueue::add(SessionPointer &session)
    {
        session_queue_.insert(std::make_pair(session->get_id(), session));
    }

    // 通过id获取Session
    SessionPointer TCPSessionQueue::get(TCPSessionID id)
    {
        auto found = session_queue_.find(id);
        if (found != session_queue_.end())
        {
            return found->second;
        }
        return SessionPointer();
    }

    // 移除指定id的Session
    void TCPSessionQueue::remove(TCPSessionID id)
    {
        session_queue_.erase(id);
    }

    // 清空整个队列
    void TCPSessionQueue::clear()
    {
        session_queue_.clear();
    }

    // 遍历Session队列
    void TCPSessionQueue::foreach(const std::function<void(const SessionPointer &session)> &cb)
    {
        auto itr = session_queue_.begin();
        while (itr != session_queue_.end())
        {
            cb(itr->second);
            ++itr;
        }
    }
}

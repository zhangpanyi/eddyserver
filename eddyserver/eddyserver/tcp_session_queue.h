#ifndef __TCP_SESSION_QUEUE_H__
#define __TCP_SESSION_QUEUE_H__

#include <unordered_map>
#include "types.h"

namespace eddyserver
{
    class TCPSessionQueue final
    {
    public:
        TCPSessionQueue() = default;

    public:
        /**
         * 获取Session数量
         */
        size_t size() const;

        /**
         * 添加Session入队列
         */
        void add(SessionPointer &session);

        /**
         * 通过id获取Session
         */
        SessionPointer get(TCPSessionID id);

        /**
         * 移除指定id的Session
         */
        void remove(TCPSessionID id);

        /**
         * 清空整个队列
         */
        void clear();

        /**
         * 遍历Session队列
         * @param cb 回调函数
         */
        void foreach(const std::function<void(const SessionPointer &session)> &cb);

    private:
        TCPSessionQueue(const TCPSessionQueue&) = delete;
        TCPSessionQueue& operator= (const TCPSessionQueue&) = delete;

    private:
        std::unordered_map<TCPSessionID, SessionPointer> session_queue_;
    };
}

#endif

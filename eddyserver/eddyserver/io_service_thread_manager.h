#ifndef __IO_SERVICE_THREAD_MANAGER_H__
#define __IO_SERVICE_THREAD_MANAGER_H__

#include <vector>
#include <unordered_map>
#include "types.h"
#include "id_generator.h"

namespace eddyserver
{
    class IOServiceThreadManager final
    {
        typedef std::unordered_map<TCPSessionID, SessionHandlePointer> SessionHandlerMap;

    public:
        explicit IOServiceThreadManager(size_t thread_num = 1);

        ~IOServiceThreadManager();

    public:
        /**
         * 运行线程
         */
        void run();

        /**
         * 停止线程
         */
        void stop();

        /**
         * 获取主线程
         */
        ThreadPointer& get_main_thread();

        /**
         * 获取负载最小的线程
         */
        ThreadPointer& get_min_load_thread();

        /**
         * 根据线程id获取线程
         */
        ThreadPointer get_thread(IOThreadID id);

        /**
         * Session连接
         */
        void on_session_connected(SessionPointer &session_ptr, SessionHandlePointer &handler);

        /**
         * Session关闭
         */
        void on_session_closed(TCPSessionID id);

        /**
         * 获取Session数量
         */
        size_t get_session_count() const;

        /**
         * 获取Session处理器
         */
        SessionHandlePointer get_session_handler(TCPSessionID id) const;

    private:
        IOServiceThreadManager(const IOServiceThreadManager&) = delete;
        IOServiceThreadManager& operator= (const IOServiceThreadManager&) = delete;

    private:
        std::vector<ThreadPointer>  threads_;
        std::vector<size_t>         thread_load_;
        SessionHandlerMap           session_handler_map_;
        IDGenerator<uint32_t>       id_generator_;
    };
}

#endif

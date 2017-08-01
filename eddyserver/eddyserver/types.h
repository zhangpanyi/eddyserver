#ifndef __EDDY_TYPES_H__
#define __EDDY_TYPES_H__

#include <memory>
#include <cstdint>
#include <functional>

namespace eddyserver
{
    typedef uint32_t                                IOThreadID;
    typedef uint32_t                                TCPSessionID;

    class                                           TCPSession;
    class                                           IOServiceThread;
    class                                           TCPSessionHandler;
    class                                           MessageFilterInterface;

    typedef std::shared_ptr<TCPSession>             SessionPointer;
    typedef std::shared_ptr<IOServiceThread>        ThreadPointer;
    typedef std::shared_ptr<TCPSessionHandler>      SessionHandlePointer;
    typedef std::shared_ptr<MessageFilterInterface> MessageFilterPointer;

    typedef std::function<SessionHandlePointer()>   SessionHandlerCreator;
    typedef std::function<MessageFilterPointer()>   MessageFilterCreator;
}

#endif

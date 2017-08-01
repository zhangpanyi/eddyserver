#ifndef __EDDYSERVER_H__
#define __EDDYSERVER_H__

namespace eddyserver
{
    class TCPClient;
    class TCPServer;
    class NetMessage;
    class MessageFilter;
    class IOServiceThread;
    class TCPSessionHandler;
    class IOServiceThreadManager;
    class MessageFilterInterface;
}

#include "eddyserver/tcp_client.h"
#include "eddyserver/tcp_server.h"
#include "eddyserver/net_message.h"
#include "eddyserver/id_generator.h"
#include "eddyserver/message_filter.h"
#include "eddyserver/tcp_session_handler.h"
#include "eddyserver/io_service_thread_manager.h"

#endif

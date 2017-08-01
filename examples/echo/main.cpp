#include <iostream>
#include <eddyserver.h>

class SessionHandle : public eddyserver::TCPSessionHandler
{
public:
    // 连接事件
    virtual void on_connected() override
    {
        std::cout << "on_connected" << std::endl;
    }

    // 接收消息事件
    virtual void on_message(eddyserver::NetMessage &message) override
    {
        std::cout << "on_message" << std::endl;
        send(message);
    }

    // 关闭事件
    virtual void on_closed() override
    {
        std::cout << "on_closed" << std::endl;
    }
};

eddyserver::MessageFilterPointer CreateMessageFilter()
{
    return std::make_shared<eddyserver::MessageFilter>();
}

eddyserver::SessionHandlePointer CreateSessionHandler()
{
    return std::make_shared<SessionHandle>();
}

int main(int argc, char *argv[])
{
    eddyserver::IOServiceThreadManager io(4);
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::from_string("127.0.0.1"), 4400);
    eddyserver::TCPServer server(ep, io, CreateSessionHandler, CreateMessageFilter);
    io.run();

    return 0;
}

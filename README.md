## 简介
基于C++14和ASIO实现高并发TCP网络框架，基于[https://code.google.com/archive/p/eddyserver/](https://code.google.com/archive/p/eddyserver/)进行改进。

## 特点
* 简单易用
* IO多线程处理，逻辑主线程处理
* 自动拆包和组包，可自定义拆包和组包策略

## 使用
```
git clone https://github.com/zhangpanyi/eddyserver
git submodule update --init --recursive
mkdir build && cd build
cmake ..
```

## 示例代码
```c++
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
```

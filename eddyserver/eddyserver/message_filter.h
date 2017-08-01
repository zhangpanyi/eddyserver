#ifndef __MESSAGE_FILTER_H__
#define __MESSAGE_FILTER_H__

#include <limits>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace eddyserver
{
    class NetMessage;

    /**
     * 消息过滤器接口
     */
    class MessageFilterInterface
    {
    public:
        typedef std::vector<uint8_t> ByteArrray;

    public:
        MessageFilterInterface() = default;

    public:
        /**
         * 表示读取任意字节数
         */
        static size_t any_bytes()
        {
            return std::numeric_limits<size_t>::max();
        }

    public:
        /**
         * 获取欲读取数据大小
         */
        virtual size_t bytes_wanna_read() = 0;

        /**
         * 获取欲写入数据大小
         * @param messages_to_be_sent 将被发送的消息列表
         */
        virtual size_t bytes_wanna_write(const std::vector<NetMessage> &messages_to_be_sent) = 0;

        /**
         * 读取数据
         * 将param1 buffer的数据写入param2 messages_received中
         * @param buffer 缓存区数据
         * @param messages_received 读取的消息列表
         * @return 读取字节数
         */
        virtual size_t read(const ByteArrray &buffer, std::vector<NetMessage> &messages_received) = 0;

        /**
         * 写入数据
         * 将param1 messages_to_be_sent的消息列表写入param2 buffer中
         * @param messages_to_be_sent 写入的消息列表
         * @param &buffer 缓存区
         * @return 写入字节数
         */
        virtual size_t write(const std::vector<NetMessage> &messages_to_be_sent, ByteArrray &buffer) = 0;

    private:
        MessageFilterInterface(const MessageFilterInterface&) = delete;
        MessageFilterInterface& operator= (const MessageFilterInterface&) = delete;
    };

    /**
     * 消息过滤器默认实现
     */
    class MessageFilter : public MessageFilterInterface
    {
    public:
        typedef uint16_t MessageHeader;
        static const size_t header_size = sizeof(MessageHeader);

    public:
        MessageFilter();

    public:
        /**
         * 获取欲读取数据大小
         */
        virtual size_t bytes_wanna_read();

        /**
         * 获取欲写入数据大小
         * @param messages_to_be_sent 将被发送的消息列表
         */
        virtual size_t bytes_wanna_write(const std::vector<NetMessage> &messages_to_be_sent);

        /**
         * 读取数据
         * 将param1 buffer的数据写入param2 messages_received中
         * @param buffer 缓存区数据
         * @param messages_received 读取的消息列表
         * @return 读取字节数
         */
        virtual size_t read(const ByteArrray &buffer, std::vector<NetMessage> &messages_received);

        /**
         * 写入数据
         * 将param1 messages_to_be_sent的消息列表写入param2 buffer中
         * @param messages_to_be_sent 写入的消息列表
         * @param &buffer 缓存区
         * @return 写入字节数
         */
        virtual size_t write(const std::vector<NetMessage> &messages_to_be_sent, ByteArrray &buffer);

    private:
        MessageHeader		header_;
        bool				header_read_;
    };
}

#endif

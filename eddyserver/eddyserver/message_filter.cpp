#include "message_filter.h"
#include <numeric>
#include <asio/ip/address_v4.hpp>
#include "net_message.h"

namespace eddyserver
{
    MessageFilter::MessageFilter()
        : header_read_(false)
    {
    }

    // 获取欲读取数据大小
    size_t MessageFilter::bytes_wanna_read()
    {
        return header_read_ ? header_ : MessageFilter::header_size;
    }

    // 获取欲写入数据大小
    size_t MessageFilter::bytes_wanna_write(const std::vector<NetMessage> &messages_to_be_sent)
    {
        if (messages_to_be_sent.empty())
        {
            return 0;
        }
        return std::accumulate(messages_to_be_sent.begin(), messages_to_be_sent.end(), 0, [](size_t sum, const NetMessage &message)
        {
            return sum + MessageFilter::header_size + message.readable();
        });
    }

    // 读取数据
    size_t MessageFilter::read(const ByteArrray &buffer, std::vector<NetMessage> &messages_received)
    {
        if (!header_read_)
        {
            uint8_t *data = const_cast<uint8_t *>(buffer.data());
            header_ = ntohs(*reinterpret_cast<MessageHeader *>(data));
            header_read_ = true;
            return MessageFilter::header_size;
        }
        else
        {
            NetMessage new_message(header_);
            new_message.write(buffer.data(), header_);
            messages_received.push_back(std::move(new_message));
            header_read_ = false;
            return header_;
        }
    }

    // 写入数据
    size_t MessageFilter::write(const std::vector<NetMessage> &messages_to_be_sent, ByteArrray &buffer)
    {
        size_t bytes = 0;
        for (size_t i = 0; i < messages_to_be_sent.size(); ++i)
        {
            const NetMessage &message = messages_to_be_sent[i];
            MessageHeader header = htons(static_cast<MessageHeader>(message.readable()));
            buffer.insert(buffer.end(),
                reinterpret_cast<const uint8_t*>(&header),
                reinterpret_cast<const uint8_t*>(&header) + sizeof(MessageHeader));
            buffer.insert(buffer.end(), message.data(), message.data() + message.readable());
            bytes += MessageFilter::header_size + message.readable();
        }
        return bytes;
    }
}

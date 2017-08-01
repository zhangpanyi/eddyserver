#ifndef __EDDY_BUFFER_H__
#define __EDDY_BUFFER_H__

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace eddyserver
{
    class NetMessage
    {
        typedef std::vector<uint8_t> DynamicVector;

    public:
        /* 动态临界值 */
        static const size_t kDynamicThreshold = 128;
        static_assert(NetMessage::kDynamicThreshold >= 0, "kDynamicThreshold must be greater than 0");

    public:
        NetMessage();

        /**
         * 构造函数
         * 预先分配内存
         * @param size 内存大小
         */
        explicit NetMessage(size_t size);

        /**
         * 构造函数
         * 将data指针后的size大小的内存写入message
         * @param data 数据地址
         * @param size 数据大小
         */
        NetMessage(const char *data, size_t size);

        /**
         * 拷贝构造函数
         */
        NetMessage(const NetMessage &other);

        /**
         * 移动构造函数
         */
        NetMessage(NetMessage &&other);

        /**
         * 重载赋值运算符
         */
        NetMessage& operator= (NetMessage &&rhs);
        NetMessage& operator= (const NetMessage &rhs);

        /**
         * 交换数据
         */
        void swap(NetMessage &other);

    public:
        /**
         * 是否是动态数组
         */
        bool is_dynmic() const
        {
            return is_dynmic_;
        }

        /**
         * 获取可读数据大小
         */
        size_t readable() const
        {
            return writer_pos_ - reader_pos_;
        }

        /**
         * 获取头部空闲大小
         */
        size_t prependable() const
        {
            return reader_pos_;
        }

        /**
         * 获取尾部空闲大小
         */
        size_t writeable() const
        {
            return (is_dynmic() ? dynamic_data_->size() : kDynamicThreshold) - writer_pos_;
        }

        /**
         * 获取容量
         */
        size_t capacity() const
        {
            return is_dynmic() ? dynamic_data_->capacity() : kDynamicThreshold;
        }

        /**
         * 是否为空
         */
        bool empty() const
        {
            return readable() == 0;
        }

        /**
         * 获取数据地址
         */
        uint8_t* data()
        {
            return (is_dynmic() ? dynamic_data_->data() : static_data_) + reader_pos_;
        }

        const uint8_t* data() const
        {
            return (is_dynmic() ? dynamic_data_->data() : static_data_) + reader_pos_;
        }

    public:
        /**
         * 清空
         */
        void clear();

        /**
         * 设为动态数组
         */
        void set_dynamic();

        /**
         * 设置容量大小（不影响数据大小）
         * @param size 数据大小
         */
        void reserve(size_t size);

        /**
         * 获取全部（只会移动读写位置）
         */
        void retrieve_all();

        /**
         * 获取数据（只会移动读写位置）
         * @param size 数据大小
         */
        void retrieve(size_t size);

        /**
         * 写入数据大小（只会移动读写位置）
         * @param size 数据大小
         */
        void has_written(size_t size);

        /**
         * 确保可写字节
         * @param size 数据大小
         */
        void ensure_writable_bytes(size_t size);

        /**
         * 读取POD类型
         */
        template <typename Type>
        Type read_pod()
        {
            static_assert(std::is_pod<Type>::value, "expects an POD type");
            assert(readable() >= sizeof(Type));
            Type value = 0;
            memcpy(&value, data(), sizeof(Type));
            retrieve(sizeof(Type));
            return value;
        }

        /**
         * 读取字符串
         */
        std::string read_string();
        void read_string(std::string *out_value);

        /**
         * 读取长度和字符串
         */
        std::string read_lenght_and_string();
        void read_lenght_and_string(std::string *out_value);

        /**
         * 写入数据
         * @param data 数据地址
         * @param size 数据大小
         */
        size_t write(const void *data, size_t size);

        /**
         * 写入POD类型
         */
        template <typename Type>
        void write_pod(Type value)
        {
            static_assert(std::is_pod<Type>::value, "expects an POD type");
            write(&value, sizeof(Type));
        }

        /**
         * 写入字符串
         */
        void write_string(const std::string &value);

        /**
         * 写入长度和字符串
         */
        void write_lenght_and_string(const std::string &value);

    private:
        /**
         * 分配空间
         * @param size 数据大小
         */
        void make_space(size_t size);

    private:
        bool                            is_dynmic_;
        size_t                          reader_pos_;
        size_t                          writer_pos_;
        std::unique_ptr<DynamicVector>  dynamic_data_;
        uint8_t                         static_data_[kDynamicThreshold];
    };

    typedef std::vector<NetMessage> NetMessageVector;
}

#endif

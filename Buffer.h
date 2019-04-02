#ifndef BBKGL_BUFFER_H
#define BBKGL_BUFFER_H

#include <algorithm>
#include <cstring>
#include <vector>
#include <cassert>

// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
//
// @code
// +-------------------+------------------+------------------+
// | prependable bytes |  readable bytes  |  writable bytes  |
// |                   |     (CONTENT)    |                  |
// +-------------------+------------------+------------------+
// |                   |                  |                  |
// 0      <=      readerIndex   <=   writerIndex    <=     size

class Buffer
{
public:
    // 提供prependable空间，让程序能以很低的代价在数据前添加几个字节
    static const size_t k_cheap_prepend = 8;
    static const size_t k_initial_size = 1024;

    Buffer() :
        buffer_(k_cheap_prepend + k_initial_size),
        reader_index_(k_cheap_prepend),
        writer_index_(k_cheap_prepend)
    {
        assert(ReadableBytes() == 0);
        assert(WritableBytes() == k_initial_size);
        assert(PrependableBytes() == k_cheap_prepend);
    }

    // 将两个Buffer对象的内容交换
    void Swap(Buffer &rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(reader_index_, rhs.reader_index_);
        std::swap(writer_index_, rhs.writer_index_);
    }

    // 可以从Buffer中读出的字节数
    size_t ReadableBytes() const { return writer_index_ - reader_index_; }
    // 可以再写入到Buffer中的字节数
    size_t WritableBytes() const { return buffer_.size() - writer_index_; }
    // 返回预留的prependable空间
    size_t PrependableBytes() const { return reader_index_; }

    // 返回Buffer中可读给用户的区域的首指针
    const char *Peek() const { return Begin() + reader_index_; }

    // 好像是从可读的区域读出len长度的字符，然后标记新的可读位置
    void Retrieve(size_t len)
    {
        assert(len <= ReadableBytes());
        reader_index_ += len;
    }

    // 让新的可读指针位置到end处
    void RetrieveUntil(const char* end)
    {
        assert(Peek() <= end);
        assert(end <= BeginWrite());
        Retrieve(end - Peek());
    }

    // 新的可读位置被初始化
    void RetrieveAll()
    {
        reader_index_ = k_cheap_prepend;
        writer_index_ = k_cheap_prepend;
    }

    // 取出所有的可读区域的字符串
    std::string RetriveAsString()
    {
        std::string str(Peek(), ReadableBytes() - 1);
        RetrieveAll();
        return str;
    }


    // 以下三个函数都用于写入数据
    void Append(const std::string& str)
    {
        Append(str.data(), str.length());
    }

    void Append(const char * data, size_t len)
    {
        EnsureWritableBytes(len);
        std::copy(data, data + len, BeginWrite());
        HasWritten(len);
    }

    void Append(const void *data, size_t len)
    {
        Append(static_cast<const char *>(data), len);
    }

    void EnsureWritableBytes(size_t len)
    {
        if (WritableBytes() < len)
            MakeSpace(len);
        assert(WritableBytes() >= len);
    }

    // 得到可写区域的首指针
    char* BeginWrite() { return Begin() + writer_index_; }
    const char* BeginWrite() const { return Begin() + writer_index_; }
    // 往Buffer中写入数据后，修改writer_index_的位置
    void HasWritten(size_t len) { writer_index_ += len; }

    // 往prependable bytes中写入数据
    void Prepend(const void *data, size_t len)
    {
        assert(len <= PrependableBytes());
        reader_index_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, Begin()+reader_index_);
    }

    // 进行扩容
    void Shrink(size_t reserve)
    {
        std::vector<char> buf(k_cheap_prepend + ReadableBytes() + reserve);
        std::copy(Peek(), Peek() + ReadableBytes(), buf.begin() + k_cheap_prepend);
        buf.swap(buffer_);
    }

    /*供外界调用的功能函数，读取socket文件内容*/
    ssize_t ReadFd(int fd, int *saved_errno);

private:
    // 获取最左端的指针
    char *Begin() { return &*buffer_.begin(); }
    const char *Begin() const { return &*buffer_.begin(); }

    // 将整体的数据往左移动reader_index_ - k_cheap_prepend个单位
    void MakeSpace (size_t len)
    {
        if (WritableBytes() + PrependableBytes() < len + k_cheap_prepend)
            buffer_.resize(writer_index_+len);
        else
        {
            // move readable data to the front, make space inside buffer
            assert(k_cheap_prepend < reader_index_);
            size_t readable = ReadableBytes();
            std::copy(Begin() + reader_index_,
                      Begin() + writer_index_,
                      Begin() + k_cheap_prepend);
            reader_index_ = k_cheap_prepend;
            writer_index_ = reader_index_ + readable;
            assert(readable == ReadableBytes());
        }
    }

    /*---------------------------------------属性方法分割线--------------------------------------------*/

    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;
};


#endif //BBKGL_BUFFER_H

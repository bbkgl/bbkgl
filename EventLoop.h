#ifndef BBKGL_EVENTLOOP_H
#define BBKGL_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <cassert>
#include <thread>

// 该类不可拷贝
class EventLoop : public boost::noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();

    // 如果当前线程不是创建对象时的线程，就报错
    void AssertInLoopThread()
    {
        if (!IsInLoopThread())
        {
            AbortNotInLoopThread();
        }
    }

    // 检查是否当前线程id和创建时记录的线程id是否相等
    bool IsInLoopThread() const { return thread_id_ == std::this_thread::get_id(); }

private:

    void AbortNotInLoopThread();

    bool looping_;
    const std::thread::id thread_id_;

};


#endif //BBKGL_EVENTLOOP_H

#ifndef BBKGL_EVENTLOOPTHREAD_H
#define BBKGL_EVENTLOOPTHREAD_H

/*
 * IO线程不一定是主线程，我们可以在任何一个线程创建并运行EventLoo。一个程序也可以不止一个IO线程
 * 我们可以按优先级将不同的scoket分给不同的线程，为了方便使用，我们定义EventLoopThread class。
 * 这正式 one loop per thread
 *
 * */

#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/core/noncopyable.hpp>

// 前置声明
class EventLoop;

class EventLoopThread : boost::noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();

    // 开始EventLoop对象的循环
    EventLoop *StartLoop();

private:
    // 开始EventLoop中的Loop循环
    void ThreadFunc();

    // 指向当前线程中的EventLoop对象
    EventLoop *loop_;

    // 是否退出当前线程
    bool exiting_;

    // 多线程三兄弟
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


#endif //BBKGL_EVENTLOOPTHREAD_H

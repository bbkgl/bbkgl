#ifndef BBKGL_EVENTLOOP_H
#define BBKGL_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <cassert>
#include <thread>
#include <vector>

class Poller;
class Channel;

// 该类不可拷贝
class EventLoop : public boost::noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    // EventLoop开始工作的函数
    void Loop();

    // EventLoop停止工作的函数，由其他线程调用
    void Quit();

    // 更新所在loop所在Channel的事件种类
    void UpdateChannel(Channel *channel);

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

    using ChannelList = std::vector<Channel *>;

    void AbortNotInLoopThread();

    // 当前循环是否已经启动
    bool looping_;

    // 是否要退出循环了
    bool quit_;

    // 线程id
    const std::thread::id thread_id_;

    // Poller智能指针，unique_ptr保证每次只有一个指针指向Poller对象
    std::unique_ptr<Poller> poller_;

    // 活跃channel列表
    ChannelList active_channels_;
};


#endif //BBKGL_EVENTLOOP_H

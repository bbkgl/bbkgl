#ifndef BBKGL_EVENTLOOP_H
#define BBKGL_EVENTLOOP_H

#include "Callbacks.h"
#include "Timestamp.h"
#include "TimerId.h"

#include <boost/noncopyable.hpp>
#include <cassert>
#include <thread>
#include <vector>

class Poller;
class Channel;
class TimerQueue;

// 该类不可拷贝
class EventLoop : public boost::noncopyable
{
public:
    //
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // EventLoop开始工作的函数
    void Loop();

    // EventLoop停止工作的函数，由其他线程调用
    void Quit();


    // 给用户新增的控制定时器的接口
    // 创建一个不能重复定期运行的定时器，让其中的回调函数在when的时间运行，返回一个定时器指针
    TimerId RunAt(const Timestamp &when, const TimerCallback &cb);
    // 创建一个不能重复定期运行的定时器，让其中的回调函数在距现在的delay时间后运行
    TimerId RunAfter(double delay, const TimerCallback &cb);
    // 创建一个可以定期重复运行的定时器，让其中的回调函数从现在开始定期运行
    TimerId RunEvery(double interval, const TimerCallback &cb);


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

    // 检查当前循环所在线程是否是创建对象时的线程
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

    // 定时器队列
    std::unique_ptr<TimerQueue> timer_queue_;

};


#endif //BBKGL_EVENTLOOP_H

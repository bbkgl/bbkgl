#ifndef BBKGL_EVENTLOOP_H
#define BBKGL_EVENTLOOP_H

#include "Callbacks.h"
#include "Timestamp.h"
#include "TimerId.h"

#include <boost/noncopyable.hpp>
#include <cassert>
#include <thread>
#include <vector>
#include <mutex>

class Epoller;
class Channel;
class TimerQueue;

// 该类不可拷贝
class EventLoop : public boost::noncopyable
{
public:
    // 接收其他线程的函数
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // EventLoop开始工作的函数
    void Loop();

    // EventLoop停止工作的函数，由其他线程调用
    void Quit();

    // 供用户传入的在EventLoop中运行的回调函数
    void RunInLoop(const Functor &cb);

    void QueueInLoop(const Functor &cb);

    // 给用户新增的控制定时器的接口
    // 创建一个不能重复定期运行的定时器，让其中的回调函数在when的时间运行，返回一个定时器指针
    TimerId RunAt(const Timestamp &when, const TimerCallback &cb);
    // 创建一个不能重复定期运行的定时器，让其中的回调函数在距现在的delay时间后运行
    TimerId RunAfter(double delay, const TimerCallback &cb);
    // 创建一个可以定期重复运行的定时器，让其中的回调函数从现在开始定期运行
    TimerId RunEvery(double interval, const TimerCallback &cb);

    // 返回poller检测到事件的时间
    Timestamp PollReturnTime() const { return poll_return_time_; }

    // 往唤醒文件中wakeup_fd_写入1，让poller_检测到，唤醒阻塞状态
    void Wakeup();

    // 更新所在loop所在Channel的事件种类
    void UpdateChannel(Channel *channel);

    // 删除某个Channel对象
    void RemoveChannel(Channel *);

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

    // 给wakeup_channel_传入作回调函数，读取经过Event::Wakeup()写入过数据的文件
    // Event::Wakeup()写入文件后会唤醒poller，这样就会检测到读事件，从而返回到Event::Loop()中
    // 由wakeup_fd_对应的wakeup_channel_执行HandleRead()函数
    void HandleRead();

    // 执行由非当前EventLoop对象的IO线程传入的回调函数
    void DoPendingFunctors();


    /*--------------------下面是属性，上面是方法-------------------*/
    // 当前循环是否已经启动
    bool looping_;

    // 是否要退出循环了
    bool quit_;

    // 线程id
    const std::thread::id thread_id_;

    // Poller智能指针，unique_ptr保证每次只有一个指针指向Poller对象
    std::unique_ptr<Epoller> poller_;

    // 活跃channel列表
    ChannelList active_channels_;

    // 定时器队列
    std::unique_ptr<TimerQueue> timer_queue_;

    // 判断当前线程是不是在运行其他线程传入EventLoop::pending_functors_的回调函数
    bool call_pending_functors_;

    // 生成的用于唤醒当前阻塞poller_的文件描述符
    int wakeup_fd_;

    // 唤醒当前阻塞poller_的channel
    std::unique_ptr<Channel> wakeup_channel_;

    // 互斥锁，保护pending_functors_增加成员
    std::mutex mutex_;

    // 接收其他线程传入的回调函数，经过RunInLoop--->QueueInLoop，然后储存在pending_functors_里
    std::vector<Functor> pending_functors_;          // 这个属性会接受其他线程提供的方法，所以需要mutex保护

    // 不知道干嘛的，应该是poller检测到事件的时间
    Timestamp poll_return_time_;
};


#endif //BBKGL_EVENTLOOP_H

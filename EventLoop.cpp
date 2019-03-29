#include "EventLoop.h"
#include <iostream>
#include <poll.h>
#include "Poller.h"
#include "Channel.h"
#include "TimerQueue.h"

// __thread变量每一个线程有一份独立实体，
// 各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
__thread EventLoop* t_loop_in_this_thread = nullptr;

// poll函数的阻塞时长为10s
const int k_poll_time_ms = 10000;

// EventLoop对象创建时，looping属性会被赋值为false，对象销毁时，会判断这个值是不是false
EventLoop::EventLoop()
        : looping_(false),
          thread_id_(std::this_thread::get_id()),
          poller_(new Poller(this)),
          quit_(false),
          timer_queue_(new TimerQueue(this))
{
    std::cout << "EventLoop created " << this << " in thread " << thread_id_ << std::endl;

    // 因为t_loop_in_this_thread一个线程是一份独立实体，初始值为空，如果一个线程创造了两个EventLoop对象
    // 那么创造了第二个对象的时候，t_loop_in_this_thread已经被第一个对象赋值，这时候就会报错，显示线程已经被占用
    if (t_loop_in_this_thread)
    {
        std::cout << "Another EventLoop " << t_loop_in_this_thread
                  << " exists in this thread " << thread_id_ << std::endl;
    }
    else
    {
        t_loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop()
{

    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
    // 如果looping在对象创建后没有被改动（即仍然为false，表示Loop()函数没有运行过）
    assert(!looping_);

    // 判断当前线程是否是创建时的线程，如果是的话，“事件循环”启动
    AssertInLoopThread();
    // “事件循环”启动
    looping_ = true;

    // 退出标记为false
    quit_ = false;

    while (!quit_)
    {
        active_channels_.clear();

        // 传地址进去，方便Epoller::poll函数对active_channels_进行改动
        poller_->Poll(k_poll_time_ms, &active_channels_);

        // 经过返回的active_channels_里面都是活跃的channel，遍历其中活跃的，然后进行事件处理
        for (ChannelList::iterator it = active_channels_.begin();
                it != active_channels_.end(); it++)
        {
            (*it)->HandleEvent();
        }
    }

    // 循环结束，停止事件循环
    std::cout << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::AbortNotInLoopThread()
{
    std::cerr << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << thread_id_
              << ", current thread id = " <<  std::this_thread::get_id() << std::endl;
}

// Channel::Update()--->EventLoop::UpdateChannel()---->Poller::UpdateChannel()
void EventLoop::UpdateChannel(Channel *channel)
{
    // 判断调用自己的是不是自己记录的EventLoop对象
    assert(channel->OwnerLoop() == this);

    // 判断当前线程是不是在创建EventLoop对象时的线程
    AssertInLoopThread();

    // 调用Poller::UpdateChannel()更新channel在Poller::pollfds_中关注的事件
    poller_->UpdateChannel(channel);
}

// 本函数应该由其他线程区调用唤醒，用来终止Loop()函数中的循环
void EventLoop::Quit()
{
    quit_ = true;
}

TimerId EventLoop::RunAt(const Timestamp &when, const TimerCallback &cb)
{
    // 根据到期时间创建一个新的定时器并让其在到期时间运行回调函数，返回其实就是定时器指针
    return timer_queue_->AddTimer(cb, when, 0.0);
}

TimerId EventLoop::RunAfter(double delay, const TimerCallback &cb)
{
    // 计算得到定时器的到期时间
    Timestamp when(addTime(Timestamp::now(), delay));
    // 根据到期时间创建一个新的定时器并让其在到期时间运行回调函数，返回其实就是定时器指针
    return RunAt(when, cb);
}

TimerId EventLoop::RunEvery(double interval, const TimerCallback &cb)
{
    // 计算得到定时器的到期时间
    Timestamp when(addTime(Timestamp::now(), interval));
    // 根据到期时间创建一个新的定时器并让其在到期时间运行回调函数，并让定时器重复定期运行，返回其实就是定时器指针
    return timer_queue_->AddTimer(cb, when, interval);
}
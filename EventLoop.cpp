#include "EventLoop.h"
#include <iostream>
#include <poll.h>
#include <sys/eventfd.h>

#include "Poller.h"
#include "Channel.h"
#include "TimerQueue.h"

// __thread变量每一个线程有一份独立实体，
// 各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
__thread EventLoop* t_loop_in_this_thread = nullptr;

// poll函数的阻塞时长为10s
const int k_poll_time_ms = 10000;

// 生成唤醒poller_的文件描述符
static int CreateEventFd()
{
    // 生成非阻塞的文件描述符
    int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0)
    {
        std::cerr << "Failed in eventfd";
        abort();
    }
    return evfd;
}

// EventLoop对象创建时，looping属性会被赋值为false，对象销毁时，会判断这个值是不是false
EventLoop::EventLoop()
        : looping_(false),
          thread_id_(std::this_thread::get_id()),
          poller_(new Poller(this)),
          quit_(false),
          timer_queue_(new TimerQueue(this)),       // 会给定时器队列分配一个文件描述符，并注册读事件
          call_pending_functors_(false),
          wakeup_fd_(CreateEventFd()),
          wakeup_channel_(new Channel(this, wakeup_fd_))  // 分配文件描述符，随后注册读事件
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

    // 将唤醒当前poller_的工作交给了wakeup_channel，传入的HandleRead()可以读取wakeup_fd_的信息
    // 猜测：这样能让poller::检测到事件发生而停止阻塞
    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_->EnableReading();
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
        poll_return_time_ = poller_->Poll(k_poll_time_ms, &active_channels_);

        // 经过返回的active_channels_里面都是活跃的channel，遍历其中活跃的，然后进行事件处理
        for (ChannelList::iterator it = active_channels_.begin();
                it != active_channels_.end(); it++)
        {
            (*it)->HandleEvent(poll_return_time_);
        }
        // 执行其他线程传入的回调函数
        DoPendingFunctors();
    }

    // 循环结束，停止事件循环
    std::cout << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

// 本函数用来终止Loop()函数中的循环
void EventLoop::Quit()
{
    quit_ = true;
    // 如果当前线程不是创建EventLoop时的线程，则唤醒poller_，不然本EventLoop持续阻塞的话，就无法正常退出了
    // poller_会告诉对应的wakeup_channel_，然后channel使用传入的回调函数终止EventLoop
    if (!IsInLoopThread())
        Wakeup();

    /*
     * 为什么EventLoop自己的IO线程调用Qiut()的时候不用唤醒呢？
     * 因为自己都能调用Quit()了。。。那就说明没有被阻塞，不用唤醒。。。
     *
     * */
}

void EventLoop::RunInLoop(const EventLoop::Functor &cb)
{
    // 如果调用本函数的线程仍然是EventLoop对象创建时的线程，则直接执行函数
    if (IsInLoopThread())
        cb();
    else// 如果调用本函数的线程是由其他线程调用，那说明是要往当前EvenLoop中传入回调函数
        QueueInLoop(cb);
}

void EventLoop::QueueInLoop(const EventLoop::Functor &cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_.push_back(cb);
    }
    // 如果调用本函数的不是当前线程或者现在正在执行由其他线程传入的回调函数
    if (!IsInLoopThread() || call_pending_functors_)
        Wakeup();
    /* EventLoop::Wakeup()函数一旦被执行，则一定是由其他线程传入回调函数以后才唤醒的
     * 这也是Wakeup设计的初衷（让其他线程去唤醒），因为EventLoop一旦阻塞，则当前线程阻塞，
     * 只能由其他线程唤醒。所以只要检测到是其他线程在调用当前EventLoop的函数，则进行唤醒。
     * 当call_pending_functors_为true的时候，说明EventLoop的IO线程执行了EventLoop::Loop()中
     * 的DoPendingFunctors()，那就说明了执行if (!IsInLoopThread() || call_pending_functors_)这个语句的
     * 一定不是EventLoop自己的IO线程（创建EventLoop对象的线程），因为EventLoop自己的IO线程正在执行
     * DoPendingFunctors()函数。所以进行唤醒...
     *
     * */
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

void EventLoop::RemoveChannel(Channel *channel)
{
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->RemoveChannel(channel);
}

void EventLoop::AbortNotInLoopThread()
{
    std::cerr << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << thread_id_
              << ", current thread id = " <<  std::this_thread::get_id() << std::endl;
}

// 唤醒函数，往唤醒的文件中写入活动的事件，让阻塞的poller_检测到
// 这样就能完成唤醒
void EventLoop::Wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one))
        std::cerr << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;

    printf("Wakeup!\n");
}

// 这个函数是要传入唤醒channel（wakeup_channel）的函数，用于读取事件
// 本函数应该在EventLoop::Wakeup()以后执行，只有往唤醒文件中写入了东西，poller_才能检测到事件
// poller_检测到事件以后，就会调用channel去读取文件，而调用的channel正是传入了EventLoop::HandleRead()回调函数的
// wakeup_channel_，然后调用本函数
void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one))
        std::cerr << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
}

// 本函数只能由IO线程调用，所以执行functors的也一定是IO线程，不存在线程安全问题
// 执行其他线程传入当前线程的函数
void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    // 标记已经开始执行其他线程传入的回调函数
    call_pending_functors_ = true;
    // 进行保护，防止其他线程再次进入
    {
        std::lock_guard<std::mutex> lock(mutex_);   // 上锁
        functors.swap(pending_functors_);           // 转移内容
    }
    // 执行其他线程传入的回调函数
    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();

    // 标记其他线程传入的回调函数已经执行结束
    call_pending_functors_ = false;
}
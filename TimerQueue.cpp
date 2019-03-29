#include <sys/timerfd.h>
#include <iostream>
#include <cstring>
#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

// 创建定时器文件描述符
int CreateTimerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerfd < 0)
    {
        std::cerr << "Failed in timerfd_create\n";
    }
    return timerfd;
}

// 计算事件when到现在的时间差
struct timespec HowMuchTimeFormNow(Timestamp when)
{
    // 计算出时间差
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    // 如果时间差太小的话，就按100微秒算
    if (microseconds < 100)
        microseconds = 100;
    // 将时间差转化成秒和微秒的单位
    struct timespec ts;
    ts.tv_sec = static_cast<time_t> (
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec =  static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

// 读取定时器文件中的内容
void ReadTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = read(timerfd, &howmany, sizeof(howmany));
    std::cout << "TimerQueue::HandleRead() " << howmany << " at " << now.toString() << std::endl;
    if (n != sizeof(howmany))
        std::cerr << "TimerQueue::handleRead() reads " << n << " byte instead of 8";
}

// 重设定时器到期时间，这个函数在TimerQueue::AddTimer()中调用，就是在插入新的定时器以后，重设定时器队列的到期时间
void ResetTimerfd(int timerfd, Timestamp expiration)
{
    // 使用timerfd_settime()函数唤醒loop
    struct itimerspec new_value;
    struct itimerspec old_value;
    bzero(&new_value, sizeof(new_value));
    bzero(&old_value, sizeof(old_value));
    new_value.it_value = HowMuchTimeFormNow(expiration);
    int ret = timerfd_settime(timerfd, 0, &new_value, &old_value);

    if (ret)
        std::cerr << "timerfd_settime";
}

TimerQueue::TimerQueue(EventLoop *loop) :
    loop_(loop),
    timerfd_(CreateTimerfd()),
    timerfd_channel_(loop, timerfd_),
    timers_()
{
    // 为定时器队列的channel设置读事件回调函数，并设置为可读事件
    timerfd_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
    timerfd_channel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    close(timerfd_);

    // 这里暂时没有使用智能指针，所以得手动回收
    for (TimerList::iterator it = timers_.begin();
            it != timers_.end(); it++)
        delete it->second;
}

TimerId TimerQueue::AddTimer(const TimerCallback &cb, Timestamp when, double interval)
{
    // 生成一个新的定时器
    Timer *timer = new Timer(cb, when, interval);

    // 这样能让最后TimerQueue::AddTimerInLoop()函数最后在IO线程的Event::Loop()循环中
    // 执行DoPendingFunctors()函数调用。。。DoPendingFunctors()函数只可能由IO线程调用
    loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
    return TimerId(timer);
}

void TimerQueue::AddTimerInLoop(Timer *timer)
{
    loop_->AssertInLoopThread();

    // 插入到定时器队列
    bool EarliestChange = Insert(timer);

    // 队列中增加新的定时器以后，改变新的到期时间
    if (EarliestChange)
        ResetTimerfd(timerfd_, timer->Expiration());
}

void TimerQueue::HandleRead()
{
    // 判断是否当前线程是创建EventLoop对象时的线程
    loop_->AssertInLoopThread();
    // 生成当前的时间
    Timestamp now(Timestamp::now());
    // 读取定时器事件
    ReadTimerfd(timerfd_, now);

    // 根据当前时间，得到过期的定时器
    std::vector<Entry> expired = GetExpired(now);

    // 遍历所有到期的定时器，让其中的函数运行处理事件
    for (std::vector<Entry>::iterator it = expired.begin();
            it != expired.end(); it++)
        it->second->Run();

    // 定时器中的函数运行以后，需要对定时器的时间重新设置
    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    // 因为set中是升序排列，取到左起第一个大于当前时间的定时器的迭代器
    TimerList::iterator it = timers_.lower_bound(sentry);
    // 如果迭代器指向的定时器对象的时间比当前时间晚，或者迭代器指到了队列尾，都是符合条件的
    assert(it == timers_.end() || now < it->first);
    // 将过期的定时器都拷贝到expired中
    std::copy(timers_.begin(), it, back_inserter(expired));
    // 然后从timers_中删除这些过期的迭代器
    timers_.erase(timers_.begin(), it);

    return expired;
}

void TimerQueue::Reset(const std::vector<Entry> &expired, Timestamp now)
{
    // 下一个到期时间
    Timestamp next_expire;

    // 陈硕教程里迭代器用的const_iterator，按照常理，这是不能修改被迭代对象的值，但是这里涉及到修改
    // 所以改成了auto
    for (auto it = expired.begin();
            it != expired.end(); it++)
    {
        // 如果这个定时器可以重复间隔执行。。。如果用户给定时器Timer::interval设置了值的话，就是可重复运行
        if (it->second->Reapeat())
        {
            // 现在就开始启动这个定时器
            it->second->Restart(now);
            // 并插入到队列里面
            Insert(it->second);
        }
        else // 否则将这个定时器删除，因为不可复用
            delete it->second;
    }
    // 如果定时器队列不为空，则获取到最早的定时器队列响应时间
    if (!timers_.empty())
        next_expire = timers_.begin()->second->Expiration();
    // 获取到的下次到期时间，如果合法，则为队列下次的到期时间
    if (next_expire.valid())
        ResetTimerfd(timerfd_, next_expire);
}


// 将新的定时器插入到队列中
bool TimerQueue::Insert(Timer *timer)
{
    // 标记插入到新的定时器到期时间是不是最早的
    bool EarliestChanged = false;

    // 得到新的定时器的到期时间
    Timestamp when = timer->Expiration();
    TimerList::iterator it = timers_.begin();
    // 如果定时器队列为空，或者队列中最早的的定时器都没新的定时器早，那说明新的定时器必然是最早的
    if (it == timers_.end() || when < it->first)
        EarliestChanged = true;
    // 根据结果来判断是否插入对应的位置是否成功
    // std::set::insert()函数的返回值是std::pair<iterator, bool>类型的，第一个表示插入位置，第二个是插入结果
    std::pair<TimerList::iterator, bool> result =
            timers_.insert(std::make_pair(when, timer));
    assert(result.second);
    return EarliestChanged;
}
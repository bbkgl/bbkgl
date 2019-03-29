#ifndef BBKGL_TIMERQUEUE_H
#define BBKGL_TIMERQUEUE_H


#include <boost/core/noncopyable.hpp>
#include <set>
#include <vector>
#include <thread>

#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

class EventLoop;
class Timer;
class TimerId;


class TimerQueue : boost::noncopyable
{
public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    // 生成一个新的定时器插入到定时器队列，when是定时器中的函数启动的时间，返回一个定时器指针
    TimerId AddTimer(const TimerCallback &cb, Timestamp when, double interval);

    // void Cancle(TimerId timer_id);

private:
    // set中的数据元素是有序且是不能重复的，为了作区分，虽然说两个定时器的时间可能重合，但是地址是不可能相同的，因为是两个不相同的对象
    // 这里的Timer*使用的是原始指针，可以考虑unique_pre，其实项目里很多地方的指针都应该用智能指针，后期会做出改变
    using Entry = std::pair<Timestamp, Timer *>;
    using TimerList = std::set<Entry>;

    // 将定时器加入到EventLoop中
    void AddTimerInLoop(Timer *timer);

    // 当定时器时间到的时候处理事件
    void HandleRead();

    // 将所有到期的定时器取出
    std::vector<Entry> GetExpired(Timestamp now);

    // 将所有到期的定时器根据当前时间进行重置，并重置定时器队列的到期时间
    void Reset(const std::vector<Entry> &expired, Timestamp now);

    // 新的定时器插入到队列
    bool Insert(Timer *timer);

    /*------------------------分割线，上面是方法，下面是属性---------------------------------*/

    // 所处的事件循环
    EventLoop *loop_;

    // 申请的文件描述符
    const int timerfd_;

    // 定时器队列所在的Channel
    Channel timerfd_channel_;

    // 根据到期时间排好序的定时器列表
    TimerList timers_;
};


#endif //BBKGL_TIMERQUEUE_H

#ifndef BBKGL_TIMER_H
#define BBKGL_TIMER_H

#include <boost/noncopyable.hpp>
#include "Callbacks.h"
#include "Timestamp.h"

// 定时器class，包括设定的时间和要运行的回调函数
// interval应该表示的是要等待的时间

class Timer : boost::noncopyable
{
public:
    Timer(const TimerCallback & cb, Timestamp when, double interval);

    void Run() const { callback_(); }

    Timestamp Expiration() const { return expiration_; }

    bool Reapeat() const {return repeat_; }

    void Restart(Timestamp now);

private:
    // 用于接受回调函数
    const TimerCallback callback_;
    // 期望的时间
    Timestamp expiration_;
    // 事件间隔
    const double intervel_;
    // 是否重复
    const bool repeat_;
};


#endif //BBKGL_TIMER_H

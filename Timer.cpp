#include "Timer.h"

Timer::Timer(const TimerCallback &cb, Timestamp when, double interval) :
    callback_(cb),
    expiration_(when),
    intervel_(interval),
    repeat_(interval > 0.0)
{}

// 如果要重新运行定时器的话，主要是看设置的间隔时间
// 如果用户没有设置间隔时间或者间隔时间小于0的话说明不能定期运行定时器了，则报错
// 如果用户设置了间隔时间的话，则预期的时间根据当前时间+间隔时间来确定下次的预期时间
void Timer::Restart(Timestamp now)
{
    if (repeat_)
        expiration_ = addTime(now, intervel_);
    else
        expiration_ = Timestamp::invalid();
}


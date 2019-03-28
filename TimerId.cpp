#include "Timer.h"

// 这个类好像就是返回

class TimerId : boost::noncopyable
{
public:
    // 必须显示构造对象
    explicit TimerId(Timer *timer) :
            value_(timer)
    {}

private:
    Timer *value_;
};

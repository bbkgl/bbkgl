#ifndef BBKGL_TIMERID_H
#define BBKGL_TIMERID_H

// 这个类好像就是简单的生成一个定时器指针

class Timer;

class TimerId
{
public:
    // 必须显示构造对象
    explicit TimerId(Timer *timer) :
            value_(timer)
    {}

private:
    Timer *value_;
};

#endif //BBKGL_TIMERID_H
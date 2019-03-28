

// 这个类好像就是返回

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

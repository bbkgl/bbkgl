//
// Created by bbkgl on 19-3-27.
//

#ifndef BBKGL_CHANNEL_H
#define BBKGL_CHANNEL_H


#include <boost/core/noncopyable.hpp>
#include <functional>

class EventLoop;

class Channel : boost::noncopyable
{
public:
    // using的作用和typedef差不多
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int fd);

    // 分发事件
    void HandleEvent();

    // 设置读回调事件
    void SetReadCallback(const EventCallback& cb) { ReadCallback_ = cb; }
    // 设置写回调事件
    void SetWriteCallback(const EventCallback& cb) { WriteCallback_ = cb; }
    // 设置错误回调事件
    void SetErrorCallback(const EventCallback& cb) { ErrorCallback_ = cb; }

    // 获取当前文件描述符
    int GetFd() const { return fd_; }

    // 获取当前事件
    int GetEvents() const { return events_; }

    // 设置活跃事件
    void SetEvents(int revt) { revents_ = revt; }

    // 判断当前Channel关注的事件是不是空事件
    bool IsNoneEvent() const { return events_ == k_none_event; }

    // 让对应种类的事件被关注，并在Poller::UpdateChannel()中更新事件列表pollfds_以及映射表
    void EnableReading() { events_ |= k_read_event; Update(); }
//    void EnableWriting() { events_ |= k_write_event; Update(); }
//    void DisableWriting() { events_ &= ~k_write_event; Update(); }
//    void DisableAll() { events_ = k_none_event; Update(); }

    // Poller中要调用的函数，返回当前channel在Poller映射表中的位置
    int GetIndex() { return index_; }
    void SetIndex(int idx) { index_ = idx; }

    // 返回当前执行的循环
    EventLoop *OwnerLoop() { return loop_; }

private:
    // 实际上是先调用EvenLoop::UpdateChannel()，最后调用Poller::UpdateChannel()
    void Update();

    // 这是因为所有Channel对象都共享事件的值
    // 无论在哪里，一种事件对应的值是一样的，设置为static可以节省空间
    // static常量的定义（与声明区分）放到源文件中
    static const int k_none_event;
    static const int k_read_event;
    static const int k_write_event;

    EventLoop *loop_;
    const int  fd_;

    // 当前关心的IO事件，经过Update()函数告诉Poller()，我关注的是什么事件，然后委托内核去检测对应的事件
    int        events_;
    // 在events列表经由Poller::Poll()函数中处理后，最后返回列表时更新的当前活跃的IO事件，由EventLoop/Poller设置
    int        revents_;
    // index由poll/epoll调用，表示的是在Epoller::pollfds_列表中的位置
    int        index_;

    EventCallback ReadCallback_;
    EventCallback WriteCallback_;
    EventCallback ErrorCallback_;
};


#endif //BBKGL_CHANNEL_H

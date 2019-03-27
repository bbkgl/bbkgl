//
// Created by bbkgl on 19-3-27.
//

#ifndef BBKGL_CHANNEL_H
#define BBKGL_CHANNEL_H


#include <boost/core/noncopyable.hpp>
#include <functional>
#include "EventLoop.h"

class Channel : boost::noncopyable
{
public:
    // using的作用和typedef差不多
    using EventCallback = std::function<void()>;

    Channel(EventLoop * loop, int fd);

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

    // 判断是不是不可用事件
    bool IsNoneEvent() const { return events_ == k_none_event; }

    // 让对应种类的事件可用，并更新事件
    void EnableReading() { events_ |= k_read_event; Update(); }
//    void EnableWriting() { events_ |= k_write_event; Update(); }
//    void DisableWriting() { events_ &= ~k_write_event; Update(); }
//    void DisableAll() { events_ = k_none_event; Update(); }

    // Poller中要调用的函数
    int GetIndex() { return index_; }
    void SetIndex(int idx) { index_ = idx; }

    // 返回当前执行的循环
    EventLoop *OwnerLoop() { return loop_; }

private:
    void Update();

    // 这是因为所有Channel对象都共享事件的值
    // 无论在哪里，一种事件对应的值是一样的，设置为static可以节省空间
    // static常量的定义（与声明区分）放到源文件中
    static const int k_none_event;
    static const int k_read_event;
    static const int k_write_event;

    EventLoop *loop_;
    const int  fd_;

    // 当前关心的IO事件
    int        events_;
    // 当前活跃的IO事件，由EventLoop/Poller设置
    int        revents_;
    // index由poll/epoll调用，表示的是在epoller的pfds_中的位置
    int        index_;

    EventCallback ReadCallback_;
    EventCallback WriteCallback_;
    EventCallback ErrorCallback_;
};


#endif //BBKGL_CHANNEL_H

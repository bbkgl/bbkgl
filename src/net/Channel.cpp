//
// Created by bbkgl on 19-3-27.
//

#include "Channel.h"
#include "poll.h"
#include "EventLoop.h"
#include <iostream>

// static常量的定义放到源文件中
const int Channel::k_none_event = 0;
const int Channel::k_read_event = POLLIN | POLLPRI;
const int Channel::k_write_event = POLLOUT;

Channel::Channel(EventLoop *loop, int fd) :
    loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    event_handling_(false)
{}

Channel::~Channel()
{
    // 断言，Channel对象不处于事件处理状态
    assert(!event_handling_);
}

// 更新事件，Update()函数会调用loop_->UpdateChannel()，然后又调用Poller::UpdateChannel()
void Channel::Update()
{
    loop_->UpdateChannel(this);
}

// Channel的核心，由EventLoop::loop()调用
void Channel::HandleEvent(Timestamp recv_time)
{
    // 标记正在处理事件
    event_handling_ = true;
    // 警告事件活跃
    if (revents_ & POLLNVAL)
        std::cout << "Channel::handle_event() POLLNVAL";

    // 对方关闭了连接
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        std::cout << "WARN! Channel::HandleEvent() POLLHUP" << std::endl;
        if (close_callback_)
            close_callback_();
    }

    // 错误事件活跃
    if (revents_ & (POLLERR | POLLNVAL))
        if (error_callback_)
            error_callback_();

    // 如果是读事件活跃
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
        if (read_callback_)
            read_callback_(recv_time);

    // 写事件活跃
    if (revents_ & POLLOUT)
        if (write_callback_)
            write_callback_();

    // 标记事件处理完成，进入非处理状态
    event_handling_ = false;
}



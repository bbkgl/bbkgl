//
// Created by bbkgl on 19-3-27.
//

#include "Channel.h"
#include "poll.h"
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
    index_(-1)
{}

// 更新事件，Update()函数会调用loop_->UpdateChannel()，然后又调用Poller::UpdateChannel()
void Channel::Update()
{
    loop_->UpdateChannel();
}

// Channel的核心，由EventLoop::loop()调用
void Channel::HandleEvent()
{
    // 警告事件活跃
    if (revents_ & POLLNVAL)
        std::cout << "Channel::handle_event() POLLNVAL";

    // 错误事件活跃
    if (revents_ & (POLLERR | POLLNVAL))
        if (ErrorCallback_)
            ErrorCallback_();

    // 如果是读事件活跃
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
        if (ReadCallback_)
            ReadCallback_();

    // 写事件活跃
    if (revents_ & POLLOUT)
        if (WriteCallback_)
            WriteCallback_();
}



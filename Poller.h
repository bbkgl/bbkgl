//
// Created by bbkgl on 19-3-27.
//

#ifndef BBKGL_POLLER_H
#define BBKGL_POLLER_H

#include <vector>
#include <boost/core/noncopyable.hpp>
#include "Timestamp.h"
#include "EventLoop.h"
#include "Channel.h"
#include <map>

// 前置声明
class Channel;
struct pollfd;

class Poller : boost::noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    ~Poller();

    // 核心函数，委托内核检测
    Timestamp Poll(int timeout_ms, ChannelList *active_channels);

    // 更新Channel的事件
    void UpdateChannel(Channel * channel);;

    // 判断是不是在创建loop的线程里
    void AssertInLoopThread() { owner_loop_->AssertInLoopThread(); }

private:
    // 遍历pollfds_，将活跃事件填入到channels列表中，由Poller::Poll()函数调用
    void FillActiveChannels(int num_events, ChannelList *active_channels) const;

    // Poller属于哪个循环
    EventLoop *owner_loop_;

    using PollFdList = std::vector<struct pollfd>;
    using ChannelMap = std::map<int, Channel*>;

    // 委托内核检测的事件列表
    PollFdList pollfds_;

    // fd到channel的映射
    ChannelMap channels_;
};


#endif //BBKGL_POLLER_H

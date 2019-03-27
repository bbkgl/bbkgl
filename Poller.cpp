//
// Created by bbkgl on 19-3-27.
//

#include "Poller.h"
#include <poll.h>
#include <iostream>

Poller::Poller(EventLoop *loop) :
    owner_loop_(loop)
{}

Poller::~Poller()
{}


// 设置等待的最大时长
// 委托内核作检测，将活跃的Channels传入到active_channels中
Timestamp Poller::Poll(int timeout_ms, Poller::ChannelList *active_channels)
{
    // 获取到活跃Channels的个数
    int num_events = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout_ms);
    // 获取当前时间类
    Timestamp now(Timestamp::now());

    // 如果活跃的Channels个数大于0，则将活跃的填充到列表中
    if (num_events > 0)
    {
        std::cout << num_events << " events happended\n";
        FillActiveChannels(num_events, active_channels);
    } else if (num_events == 0)
        std::cout << " nothing happended\n";
    else
        std::cerr << "Poller::Poll Error\n";
    // 返回当前时间
    return now;
}

// 遍历pollfds_，将活跃事件填入到channels列表中
void Poller::FillActiveChannels(int num_events, Poller::ChannelList *active_channels) const
{
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end(); pfd++)
    {
        // 如果存在活跃事件
        if (pfd->revents > 0)
        {
            --num_events;
            // 如果这个事件活跃，则在映射表中找到对应的channel
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            // 假设找到了
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            // 假设真的找到了？？？
            assert(channel->GetFd() == pfd->fd);
            // 给Channel设置活跃事件
            channel->SetEvents(pfd->events);
            // 加入活跃channels表
            active_channels->push_back(channel);
        }
    }
}

void Poller::UpdateChannel(Channel *channel)
{
    // 判断当前线程是否是创建时的线程
    AssertInLoopThread();

    // 打印当前的文件描述符对应的事件
    std::cout << "fd = " << channel->GetFd() << " events = " << channel->GetEvents();
    // 如果响应的是一个新的channel，需要即使更新poller中的channels_和pollfds_
    if (channel->GetIndex() < 0)
    {
        // 假设找到了
        assert(channels_.find(channel->GetFd()) != channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->GetFd();
        // 新的结构体标记事件
        pfd.events = static_cast<short>(channel->GetEvents());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->SetIndex(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        // 假设找到了
        assert(channels_.find(channel->GetFd()) != channels_.end());
        // 假设真的找到了
        assert(channels_[channel->GetFd()] == channel);
        int idx = channel->GetIndex();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->GetFd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->GetEvents());
        pfd.revents = 0;

        // 如果channel暂时不关心任何事件，酒吧pollfd.fd设为-1，让poller忽略这个channel
        if (channel->IsNoneEvent())
        {
            pfd.fd = -1;
        }
    }
}
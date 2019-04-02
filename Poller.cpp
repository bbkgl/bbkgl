//
// Created by bbkgl on 19-3-27.
//

#include <poll.h>
#include <iostream>
#include <algorithm>
#include "Channel.h"
#include "Poller.h"

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

// 遍历pollfds_，将活跃事件填入到channels列表中，由Poller::Poll()函数调用
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

// 本UpdateChannel()首先由Channel本身的Update调用，在channel自身关注的事件发生改变以后
// 其在Poller中对应的的事件也要改变
void Poller::UpdateChannel(Channel *channel)
{
    // 判断当前线程是否是创建时的线程
    AssertInLoopThread();

    // 打印当前的文件描述符对应的事件
    std::cout << "[Poller::UpdateChannel()] fd = " << channel->GetFd() << " events = " << channel->GetEvents() << std::endl;
    // 如果响应的是一个新的channel，需要即使更新poller中的channels_和pollfds_
    if (channel->GetIndex() < 0)
    {
        // 假设找不到，即这是个新的Channel，channels_中应该不存在
        assert(channels_.find(channel->GetFd()) == channels_.end());
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
    else   // 如果是已经存在的channel....因为本身channel传的是指针，
           // 所以当channel对象之前已经修改时，对应的映射表channels不用修改，只需要修改与之对应的Poller::pollfds_表
    {
        // 假设在Poller::channels中找到了
        assert(channels_.find(channel->GetFd()) != channels_.end());
        // 假设真的找到了
        assert(channels_[channel->GetFd()] == channel);
        // 获取在Poller::PollFdList中的位置
        int idx = channel->GetIndex();
        // 在pollfds_真的存在
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        // 找到channel对应的pollfd（注意这里是引用）
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->GetFd() || pfd.fd == -channel->GetFd() - 1);
        // 更改channel对应的pollfd对应的关心的事件
        pfd.events = static_cast<short>(channel->GetEvents());

        // 因为活跃事件只能被内核检测后改变，所有在当前的pfd更新后将revents设置为0，表示暂时不活跃
        pfd.revents = 0;

        // 如果channel暂时不关心任何事件，就把pollfd.fd设为-1，让poller忽略这个channel
        if (channel->IsNoneEvent())
        {
            pfd.fd = -channel->GetFd()-1;
        }
    }
}

void Poller::RemoveChannel(Channel *channel)
{
    AssertInLoopThread();
    std::cout << "fd = " << channel->GetFd() << std::endl;

    // 以下步骤在pollfds_和channels_中找到channel
    // 找到了
    assert(channels_.find(channel->GetFd()) != channels_.end());
    assert(channels_[channel->GetFd()] == channel);
    // 确实channel已经执行过DisableAll了
    assert(channel->IsNoneEvent());
    int idx = channel->GetIndex();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd &pfd = pollfds_[idx];
    (void)pfd;
    assert(pfd.fd == -channel->GetFd()-1 && pfd.events == channel->GetEvents());

    // channels_中删除
    size_t n = channels_.erase(channel->GetFd());

    assert(n == 1);
    (void)n;

    if (idx == pollfds_.size() - 1)
        pollfds_.pop_back();
    else
    {
        int channel_at_end = pollfds_.back().fd;
        // 将找到的元素位置和最后的元素交换
        std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channel_at_end < 0)
            channel_at_end = -channel_at_end - 1;
        // channels_设置新的下一个元素索引
        channels_[channel_at_end]->SetIndex(idx);
        // pop掉最后一个元素
        pollfds_.pop_back();
    }
}
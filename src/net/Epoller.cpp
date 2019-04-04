//
// Created by bbkgl on 19-4-4.
//

#include "Epoller.h"
#include "Channel.h"
#include <sys/poll.h>
#include <iostream>
#include <cstring>

// 在linux中，epoll()和poll()的常量都是相等的
// 比如EPOLLIN == POLLIN等

namespace
{
    const int k_new = -1;
    const int k_added = 1;
    const int k_deleted = 2;
}

Epoller::Epoller(EventLoop *loop) :
    owner_loop_(loop),
    epollfd_(::epoll_create(EPOLL_CLOEXEC)),
    events_(k_init_event_list_size)
{
    if (epollfd_ < 0)
        std::cerr << "Epoller::Epoller()" << std::endl;
}

Epoller::~Epoller()
{
    // 关闭首结点套接字
    close(epollfd_);
}

Timestamp Epoller::Poll(int timout, Epoller::ChannelList *active_channels)
{
    // 活跃事件的
    int num_events = epoll_wait(epollfd_,
                                events_.data(),
                                static_cast<int>(events_.size()),
                                timout);
    Timestamp now(Timestamp::now());
    if (num_events > 0)
    {
        std::cout << num_events << "events happend" << std::endl;
        FillActiveChannels(num_events, active_channels);
        if (static_cast<size_t>(num_events) == events_.size())
            events_.resize(events_.size() * 2);
    }
    else if (num_events == 0)
        std::cout << " nothing happended\n";
    else
        std::cerr << "Epoller::poll()\n";
    return now;
}

void Epoller::FillActiveChannels(int num_events, Epoller::ChannelList *active_channels) const
{
    assert(static_cast<size_t>(num_events) <= events_.size());
    for (int i = 0; i < num_events; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->GetFd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
#endif
        channel->SetEvents(events_[i].events);
        active_channels->push_back(channel);
    }
}

void Epoller::UpdateChannel(Channel *channel)
{
    AsserInLoopThread();
    std::cout << "fd = " << channel->GetFd() << " events = " << channel->GetEvents() << std::endl;
    const int index = channel->GetIndex();
    if (index == k_new || index == k_deleted)  // 如果这个channel是新的或者是已经被删除的
    {
        int fd = channel->GetFd();
        if (index == k_new)          // 如果是新的
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else                         // 如果是以前被删除过的
        {
            assert(channels_.find(fd) == channels_.end());
            assert(channels_[fd] == channel);
        }
        // 增加事件
        channel->SetIndex(k_added);
        Update(EPOLL_CTL_ADD, channel);
    }
    else   // 如果是已经在里面的，那就是要对事件进行修改了
    {
        int fd =channel->GetFd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == k_added);
        if (channel->IsNoneEvent())            // 如果该channel要取消注册事件
        {
            Update(EPOLL_CTL_DEL, channel);
            channel->SetIndex(k_deleted);      // 标记被删除
        }
        else
            Update(EPOLL_CTL_MOD, channel);
    }
}

void Epoller::RemoveChannel(Channel *channel)
{
    AsserInLoopThread();
    int fd = channel->GetFd();
    std::cout << "rm fd = " << fd << std::endl;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());
    int index = channel->GetIndex();
    assert(index == k_added || index == k_deleted);
    size_t n = channels_.erase(fd);     // 从映射表中删除
    (void)n;
    assert(n == 1);

    // 如果要删除的事件之前的状态是已经添加的话，得从epoll事件表中也删除
    if (index == k_added)
        Update(EPOLL_CTL_DEL, channel);
    channel->SetIndex(k_deleted);
}

void Epoller::Update(int operation, Channel *channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->GetEvents();
    event.data.ptr = channel;
    int fd = channel->GetFd();
    if (epoll_ctl(epollfd_, operation, fd, &event) < 0);
    {
        if (operation == EPOLL_CTL_DEL)
            std::cerr << "syserror: epoll_ctl op = " << " fd = " << fd << std::endl;
        else
            std::cerr << "oterr: epoll_ctl op = " << " fd = " << fd << std::endl;
    }
}

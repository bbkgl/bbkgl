#ifndef BBKGL_EPOLLER_H
#define BBKGL_EPOLLER_H

#include <boost/noncopyable.hpp>
#include <vector>
#include <map>
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Timestamp.h"

class Channel;

class Epoller : boost::noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;
    using ChannelMap = std::map<int, Channel *>;
    using EventList = std::vector<epoll_event>;

    Epoller(EventLoop *loop);
    ~Epoller();

    Timestamp Poll(int timout, ChannelList *active_channels);

    void UpdateChannel(Channel *channel);

    void RemoveChannel(Channel *channel);

    void AsserInLoopThread() { owner_loop_->AssertInLoopThread(); }


private:
    static const int k_init_event_list_size = 16;

    void FillActiveChannels(int num_events, ChannelList *active_channels) const;

    void Update(int operation, Channel *channel);

    // 23333
    EventLoop *owner_loop_;
    int epollfd_;
    EventList events_;
    ChannelMap channels_;
};


#endif //BBKGL_EPOLLER_H

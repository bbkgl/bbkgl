//
// Created by bbkgl on 19-3-27.
//

#ifndef BBKGL_POLLER_H
#define BBKGL_POLLER_H

#include <vector>
#include <boost/core/noncopyable.hpp>
#include "Timestamp.h"

// 前置声明
class Channel;

class Poller : boost::noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller();
    ~Poller();
    


private:


};


#endif //BBKGL_POLLER_H

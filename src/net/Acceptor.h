#ifndef BBKGL_ACCEPTOR_H
#define BBKGL_ACCEPTOR_H

#include <functional>
#include <boost/noncopyable.hpp>
#include "Channel.h"
#include "Socket.h"

class InetAddress;
class EventLoop;

class Acceptor : boost::noncopyable
{
public:
    using NewConnetionCallback = std::function<void(int sockfd, const InetAddress &)>;

    // 根据所在的EventLoop和服务器的主机信息构建Acceptor
    Acceptor(EventLoop *loop, const InetAddress &listen_addr);

    void SetNewConnetionCallback(const NewConnetionCallback &cb) { new_conn_callback_ = cb; }

    bool Listenning() const { return listenning_; }

    void Listen();

private:
    void HandleRead();

    EventLoop *loop_;

    Socket accept_socket_;

    Channel accept_channel_;

    NewConnetionCallback new_conn_callback_;

    bool listenning_;
};


#endif //BBKGL_ACCEPTOR_H

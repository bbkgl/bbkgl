#ifndef BBKGL_SOCKET_H
#define BBKGL_SOCKET_H

#include <boost/noncopyable.hpp>

class InetAddress;

class Socket : boost::noncopyable
{
public:
    explicit Socket(int sockfd) :
            sockfd_(sockfd)
    {}

    ~Socket();

    int GetFd() const { return sockfd_; }

    void BindAddress(const InetAddress &localaddr);

    void Listen();

    int Accept(InetAddress *peeraddr);

    void SetReUseAddr(bool on);


private:
    const int sockfd_;
};


#endif //BBKGL_SOCKET_H

#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOpts.h"
#include <cstring>
#include <netinet/tcp.h>

Socket::~Socket()
{
    sockets::Close(sockfd_);
}

void Socket::BindAddress(const InetAddress &localaddr)
{
    sockets::BindOrDie(sockfd_, localaddr.GetSockAddrInet());
}

void Socket::Listen()
{
    sockets::ListenOrDie(sockfd_);
}

int Socket::Accept(InetAddress *peeraddr)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int connfd = sockets::Accept(sockfd_, &addr);
    if (connfd >= 0)
        peeraddr->SetSockAddrInet(addr);
    return connfd;
}

void Socket::SetReUseAddr(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
}

void Socket::ShutdownWrite()
{
    sockets::ShutdownWrite(sockfd_);
}

// 禁用Nagle算法降低延迟
void Socket::SetTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}




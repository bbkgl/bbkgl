#include "SocketsOpts.h"

#include <iostream>
#include <cerrno>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <boost/implicit_cast.hpp>

using SA = struct sockaddr;

const SA *sockaddr_cast(const struct sockaddr_in *addr)
{
    return static_cast<const SA *>(boost::implicit_cast<const void *>(addr));
}

SA *sockaddr_cast(struct sockaddr_in *addr)
{
    return static_cast<SA *>(boost::implicit_cast<void *>(addr));
}

// 给文件描述符设置成非阻塞模式
void SetNonblockAndCloseOnExec(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = fcntl(sockfd, F_SETFL, flags);

    flags = fcntl(sockfd, F_GETFL, 0);
    flags |= FD_CLOEXEC;
    ret = fcntl(sockfd, F_SETFL, flags);
}

int CreateNonblockingOrDie()
{
#if VALGRIND
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
        std::cerr << "SocketsOpts---CreateNonblockingOrDie\n";
    SetNonblockAndCloseOnExec(sockfd);
#else
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
        std::cerr << "SocketsOpts---CreateNonblockingOrDie\n";
#endif
    return sockfd;
}

void BindOrDie(int sockfd, const struct sockaddr_in &addr)
{
    int ret = bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
    if (ret < 0)
        std::cerr << "SocketsOpts---BindOrDie\n";
}

void ListenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
        std::cerr << "SocketsOpts---ListenOrDie\n";
}

int Accept(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addr_len = sizeof(*addr);
#if VALGRIND
    int connfd = accept(sockfd, sockaddr_cast(addr), &addrlen);
    SetNonBlockAndCloseOnExec(connfd);
#else
    int connfd = accept4(sockfd, sockaddr_cast(addr), &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);

#endif
    if (connfd < 0)
    {
        int saved_errno = errno;
        std::cerr << "SocketsOpts---Accept\n";
        switch (saved_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                std::cerr << "unexpected error of SocketsOpts---Accept\n";
                break;
            default:
                std::cerr << "unknown error of SocketsOpts---Accept\n";
                break;
        }
    }
    return connfd;
}

void Close(int sockfd)
{
    if (close(sockfd) < 0)
        std::cerr << "SocketsOpts---Close\n";
}

void ToHostPort(char *buf, size_t size, const struct sockaddr_in &addr)
{
    char host[INET_ADDRSTRLEN] = "INVALId";
    inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
    uint16_t port = NetworkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

void FromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = HostToNetwork16(port);
    if (inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
        std::cerr << "SocketsOpts---FromHostPort\n";
}




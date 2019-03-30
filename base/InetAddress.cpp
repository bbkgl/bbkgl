#include "InetAddress.h"
#include "SocketsOpts.h"
#include <cstring>
#include <boost/static_assert.hpp>

static const in_addr_t k_inaddr_any = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_, sizeof(addr_));

    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = sockets::HostToNetwork32(k_inaddr_any);
    addr_.sin_port = sockets::HostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    sockets::FromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::ToHostPort() const
{
    char buf[32];
    sockets::ToHostPort(buf, sizeof buf, addr_);
    return buf;
}

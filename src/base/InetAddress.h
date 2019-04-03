#ifndef BBKGL_INETADDRESS_H
#define BBKGL_INETADDRESS_H


#include <cstdint>
#include <string>
#include <arpa/inet.h>

class InetAddress
{
public:
    explicit InetAddress(uint16_t port);

    InetAddress(const std::string &ip, uint16_t port);

    InetAddress(const struct sockaddr_in &addr) :
            addr_(addr)
    {}

    std::string ToHostPort() const;

    const struct sockaddr_in& GetSockAddrInet() const { return addr_; }
    void SetSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

private:

    sockaddr_in addr_;
};


#endif //BBKGL_INETADDRESS_H

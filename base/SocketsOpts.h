#ifndef BBKGL_SOCKETSOPTS_H
#define BBKGL_SOCKETSOPTS_H

#include <cstdint>
#include <endian.h>
#include <cstdio>
#include <arpa/inet.h>

namespace sockets
{
    inline uint64_t HostToNetwork64(uint64_t host64)
    {
        return htobe64(host64);
    }

    inline uint32_t HostToNetwork32(uint32_t host32)
    {
        return htobe32(host32);
    }


    inline uint16_t HostToNetwork16(uint16_t host16)
    {
        return htobe16(host16);
    }

    inline uint64_t NetworkToHost64(uint64_t net64)
    {
        return be64toh(net64);
    }

    inline uint32_t NetworkToHost32(uint32_t net32)
    {
        return be32toh(net32);
    }

    inline uint16_t NetworkToHost16(uint16_t net16)
    {
        return be16toh(net16);
    }

/*------------------------------------分割线--------------------------------------*/

    int CreateNonblockingOrDie();

// 注意这里是传引用
    void BindOrDie(int sockfd, const struct sockaddr_in &addr);

    void ListenOrDie(int sockfd);

// 注意这里是传指针
    int Accept(int sockfd, struct sockaddr_in *addr);

    void Close(int sockfd);

// 将sockaddr_in转换成"ip:port"的字符串形式，由buf接收
// 注意这里是传引用
    void ToHostPort(char *buf, size_t size, const struct sockaddr_in &addr);

// 将"ip:port"字符串形式转换成sockaddr_in结构体
    void FromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

    struct sockaddr_in GetLocalAddr(int sockfd);
}

#endif //BBKGL_SOCKETSOPTS_H

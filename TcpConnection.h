//
// Created by bbkgl on 19-3-31.
//

#ifndef BBKGL_TCPCONNECTION_H
#define BBKGL_TCPCONNECTION_H

#include <boost/core/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>
#include "Callbacks.h"
#include "base/InetAddress.h"
#include "Buffer.h"

class Channel;
class EventLoop;
class Socket;

class TcpConnection : boost::noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddress &local_addr,
                  const InetAddress &peer_addr);

    ~TcpConnection();

    EventLoop *loop;

    EventLoop *GetLoop() const { return loop_; }

    const std::string &GetName() { return name_; }

    const InetAddress &GetLocalAddress () { return local_address_; }

    const InetAddress &GetPeerAddress () { return peer_address_; }

    bool Connected() const { return state_ == k_connected; }

    // TcpServer创建后，用户设置回调函数，然后传入调用
    void SetConnCallback(const ConnectionCallback &cb) { conn_callback_ = cb; }

    // TcpServer创建后，用户设置回调函数，然后传入调用
    void SetMsgCallback(const MessageCallback &cb) { msg_callback_ = cb; }

    // 由HandleRead()调用，HandleRead()会被传入到Channel中
    void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }

    // 在TcpServer::NewConnection()中被调用，也就是说是建立连接后的后续工作
    void ConnEstablished();

    // 该函数传入到EventLoop::QueueInLoop()中最终回到IO线程调用
    void ConnDestroyed();

    // 这就是建立连接后的通信工作了
    void HandleRead(Timestamp recv_time);
    void HandleWrite();
    void HandleError();
    void HandleClose();

private:
    enum StateE{k_connecting, k_connected, k_disconnected};

    void SetState(StateE s) { state_ = s; }

    // 不谈了
    EventLoop *loop_;

    // 连接的名字，用于建立在TcpServer中的映射
    std::string name_;

    // 应该是要连接的状态，k_connecting表示正在连接中，k_connected是连接已经成功建立了
    StateE state_;

    // 本身TcpConnection的对象就是由Acceptor接收到的连接返回后构造，而这个socket_指针就是指向
    // Acceptor接收连接请求后，返回的文件描述符fd会用来构造TcpConnection，TcpConnection的构造函数
    // 会用fd构造一个Socket对象，然后socket_再指向这个Socket对象
    std::unique_ptr<Socket> socket_;

    // Channel本身就代表事件，每一个TcpConnect要和客户端实现通信的话，
    // 也就需要注册一个Channel，并设置可读（注册进poller的列表里），来实现通信
    // Channel由所在的EventLoop和建立连接后得到fd构造
    std::unique_ptr<Channel> channel_;

    // 服务器本身的host
    InetAddress local_address_;

    // 建立连接的对方的host
    InetAddress peer_address_;

    /*-----------------------------------回调函数分割线------------------------------------*/
    /*回调函数的调用由state_枚举类决定*/

    // 用于建立连接的回调函数，由用户设置，用于成功建立连接后，作出相应的操作
    ConnectionCallback conn_callback_;

    // 用于通信的回调函数，由用户设置，在已经建立的连接的客户端发来信息后，作出相应的操作
    MessageCallback msg_callback_;

    // 用于关闭连接的回调函数
    CloseCallback close_callback_;

    Buffer input_buffer_;

};

#endif //BBKGL_TCPCONNECTION_H

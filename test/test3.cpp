#include <iostream>
#include <unistd.h>
#include <thread>
#include <sys/timerfd.h>
#include <cstring>
#include "EventLoop.h"
#include "Channel.h"

EventLoop *g_loop;

// 读事件回调函数。。。也就是最后EventLoop::HandleEvent()执行的函数
void timeout()
{
    printf("Timeout!\n");
    g_loop->Quit();
}

int test3()
{
    // g_loop指针调用EventLoop::Quit()函数退出Loop()循环
    EventLoop loop;
    g_loop = &loop;

    // 申请一个计时器的文件描述符
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    // 创建一个EventLoop循环下的Channel对象，并关注timerfd定时器文件描述符
    Channel channel(&loop, timerfd);

    // 给Channel对象传入读事件回调函数
    channel.SetReadCallback(timeout);

    // 修改关注的事件为可读事件
    channel.EnableReading();

    // 对对应的定时器设置事件响应时间
    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;

    // 定时器事件立即响应
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    // 开始循环
    loop.Loop();

    close(timerfd);

    return 0;
}
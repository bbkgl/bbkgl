#include "EventLoop.h"
#include <cstdio>
#include <thread>

// 全局EventLoop指针
EventLoop *g_loop;

void Print() {}

void ThreadFunc()
{
    g_loop->RunAfter(1.0, Print);
}

int main()
{
    EventLoop loop;

    g_loop = &loop;

    std::thread t(ThreadFunc);

    loop.Loop();

    return 0;
}
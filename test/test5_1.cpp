/*
 * 本程序初衷是演示一个反例，即在TimerQueue::AddTimer()线程不安全的情况下,
 * 让其他线程去执行TimerQueue::AddTimer()，会报错。
 * 现在AddTimer已经分成了两个函数TimerQueue::AddTimer()和TimerQueue::AddTimerInLoop()
 * 这样使得TimerQueue::AddTimerInLoop()只能是在IO线程中执行，不会存在线程不安全问题。。。
 *
 * */

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

    // 其他非IO线程执行ThreadFunc()，最后调用TimerQueue::AddTimer()和TimerQueue::AddTimerInLoop()，
    // 修改前，存在线程不安全问题。修改后无线程不安全问题。
    std::thread t(ThreadFunc);

    loop.Loop();

    return 0;
}
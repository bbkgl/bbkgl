#include "EventLoopThread.h"
#include "EventLoop.h"

// 可以看到，每创建一个EventLoopThread对象就会创建一个EventLoopThread::thread_线程
// EventLoopThread::thread_线程会传入EventLoopThread::ThreadFunc()方法，生成一个新的EventLoop对象
// 新的EventLoop对象生成后会先释放EventLoopThread中的锁，然后启动Loop()循环
// 最后再调用EventLoopThread::StartLoop()返回刚刚生成的EventLoop对象指针
EventLoopThread::EventLoopThread() :
    loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::ThreadFunc, this)),
    mutex_(),
    cond_()
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    loop_->Quit();
    thread_.join();
}

EventLoop *EventLoopThread::StartLoop()
{
    // 互斥锁与条件变量配合使用，当新线程运行起来后才能够得到loop的指针
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 只要没有给loop_指针分配对象，就一直握着锁不放
        while (loop_ == nullptr)
            cond_.wait(lock);
    }
    return loop_;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();              // 释放所有的锁
    }
    loop.Loop();                         // 执行Loop循环
}
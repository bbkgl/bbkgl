//
// Created by bbkgl on 19-4-3.
//

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop) :
    base_loop_(loop),
    started_(false),
    num_threads_(0),
    next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // 这里不需要删除base_loop_，是栈上的变量
}

void EventLoopThreadPool::Start()
{
    assert(!started_);
    base_loop_->AssertInLoopThread();

    // 标记线程池开始运行
    started_ = true;

    // 产生固定数目的已经占用线程的EventLoop对象
    for (int i = 0; i < num_threads_; i++)
    {
        EventLoopThread *t = new EventLoopThread;
        threads_.push_back(t);
        loops_.push_back(t->StartLoop());
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop()
{
    base_loop_->AssertInLoopThread();
    EventLoop *loop = base_loop_;

    // Round-robin调度算法
    // 轮询每个EventLoop，然后返回去处理连接
    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
            next_ = 0;
    }
    return loop;
}
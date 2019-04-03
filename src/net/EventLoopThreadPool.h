//
// Created by bbkgl on 19-4-3.
//

#ifndef BBKGL_EVENTLOOPTHREADPOOL_H
#define BBKGL_EVENTLOOPTHREADPOOL_H


#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable
{
public:
    EventLoopThreadPool(EventLoop *loop);
    ~EventLoopThreadPool();

    // 设置线程池的线程最大数量
    void SetThreadNum(int num_threads) { num_threads_ = num_threads; }

    // 线程池开始运行
    void Start();

    // 获取到下一个已经分配了线程的EventLoop对象
    EventLoop *GetNextLoop();

private:

    // 本线程池所在的EventLoop
    EventLoop *base_loop_;
    bool started_;
    int num_threads_;
    int next_;
    boost::ptr_vector<EventLoopThread> threads_;
    std::vector<EventLoop *> loops_;
};


#endif //BBKGL_EVENTLOOPTHREADPOOL_H
